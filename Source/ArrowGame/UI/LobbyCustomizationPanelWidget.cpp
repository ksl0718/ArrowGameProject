#include "LobbyCustomizationPanelWidget.h"

#include "CustomizePartSelectorWidget.h"
#include "ArrowGame/Customization/CharacterCustomizeComponent.h"
#include "ArrowGame/Customization/CharacterPartCatalog.h"
#include "ArrowGame/Customization/CharacterPartData.h"
#include "Components/Image.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureResource.h"
#include "ArrowGame/Core/ArrowPlayerState.h"

void ULobbyCustomizationPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetupLocalPreview();
	FindPreviewActor();
	InitializeSelectors();
}

void ULobbyCustomizationPanelWidget::NativeDestruct()
{
	if (bSpawnedPreviewCaptureActor && PreviewCaptureActor)
	{
		PreviewCaptureActor->Destroy();
	}
	PreviewCaptureActor = nullptr;
	PreviewCaptureComponent = nullptr;
	bSpawnedPreviewCaptureActor = false;

	if (bSpawnedPreviewActor && PreviewActor)
	{
		PreviewActor->Destroy();
	}
	PreviewActor = nullptr;
	bSpawnedPreviewActor = false;

	Super::NativeDestruct();
}

void ULobbyCustomizationPanelWidget::SetPreviewActor(AActor* InPreviewActor)
{
	PreviewActor = InPreviewActor;
	bSpawnedPreviewActor = false;
}

bool ULobbyCustomizationPanelWidget::GetCurrentPreviewPreset(FCharacterCustomizePreset& OutPreset) const
{
	if (!PreviewActor)
	{
		return false;
	}

	const UCharacterCustomizeComponent* CustomizeComponent = PreviewActor->FindComponentByClass<UCharacterCustomizeComponent>();
	if (!CustomizeComponent)
	{
		return false;
	}

	// 프리뷰 액터가 들고 있는 선택 결과만 반환한다.
	// PlayerState에 저장할지, 그냥 미리보기로 둘지는 호출한 쪽에서 결정한다.
	OutPreset = CustomizeComponent->GetCurrentPreset();
	return true;
}

bool ULobbyCustomizationPanelWidget::CommitCurrentPresetToPlayerState() const
{
	FCharacterCustomizePreset CurrentPreset;
	if (!GetCurrentPreviewPreset(CurrentPreset))
	{
		return false;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer)
	{
		return false;
	}

	AArrowPlayerState* ArrowPlayerState = OwningPlayer->GetPlayerState<AArrowPlayerState>();
	if (!ArrowPlayerState)
	{
		return false;
	}

	// PlayerState는 서버가 최종 상태를 소유한다.
	// 로컬 UI는 현재 선택값을 서버 RPC로 제출하고, 이후 복제된 값을 실제 캐릭터가 읽어간다.
	ArrowPlayerState->ServerSetCustomizePreset(CurrentPreset);
	return true;
}

void ULobbyCustomizationPanelWidget::SetupLocalPreview()
{
	CreateLocalPreviewRenderTarget();
	SpawnLocalPreviewActor();
	SpawnLocalPreviewCaptureActor();
	SetupPreviewImage();
	CapturePreview();
}

void ULobbyCustomizationPanelWidget::CreateLocalPreviewRenderTarget()
{
	const int32 ClampedSize = FMath::Clamp(PreviewRenderTargetSize, 256, 2048);

	PreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (!PreviewRenderTarget)
	{
		return;
	}

	// 공용 RenderTarget 에셋을 쓰면 리슨 서버 PIE에서 서버 화면과 클라이언트 화면이 섞인다.
	// 그래서 위젯마다 런타임 RenderTarget을 새로 만들어 자기 화면에만 연결한다.
	PreviewRenderTarget->ClearColor = FLinearColor::Transparent;
	PreviewRenderTarget->InitCustomFormat(ClampedSize, ClampedSize, PF_FloatRGBA, false);
	PreviewRenderTarget->UpdateResourceImmediate(true);
}

void ULobbyCustomizationPanelWidget::SpawnLocalPreviewActor()
{
	if (!PreviewActorClass)
	{
		return;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer || !OwningPlayer->IsLocalController())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwningPlayer;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 프리뷰 캐릭터는 UI 전용 상태라 네트워크 복제 대상이 아니다.
	PreviewActor = World->SpawnActor<AActor>(PreviewActorClass, PreviewActorSpawnTransform, SpawnParameters);
	if (PreviewActor)
	{
		bSpawnedPreviewActor = true;
		PreviewActor->SetReplicates(false);
		PreviewActor->SetReplicateMovement(false);
	}
}

void ULobbyCustomizationPanelWidget::SpawnLocalPreviewCaptureActor()
{
	if (!PreviewCaptureActorClass || !PreviewRenderTarget)
	{
		return;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer || !OwningPlayer->IsLocalController())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwningPlayer;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewCaptureActor = World->SpawnActor<AActor>(PreviewCaptureActorClass, PreviewCaptureSpawnTransform, SpawnParameters);
	if (!PreviewCaptureActor)
	{
		return;
	}

	bSpawnedPreviewCaptureActor = true;
	PreviewCaptureActor->SetReplicates(false);
	PreviewCaptureActor->SetReplicateMovement(false);

	PreviewCaptureComponent = PreviewCaptureActor->FindComponentByClass<USceneCaptureComponent2D>();
	if (!PreviewCaptureComponent)
	{
		return;
	}

	PreviewCaptureComponent->TextureTarget = PreviewRenderTarget;
	PreviewCaptureComponent->CaptureSource = SCS_SceneColorHDR;
	PreviewCaptureComponent->bCaptureEveryFrame = true;
	PreviewCaptureComponent->bCaptureOnMovement = false;

	if (PreviewActor)
	{
		// 캡처에는 프리뷰 액터만 넣어서 RenderTarget 배경을 투명하게 유지한다.
		PreviewCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		PreviewCaptureComponent->ShowOnlyActors.Reset();
		PreviewCaptureComponent->ShowOnlyActors.Add(PreviewActor);
	}
}

void ULobbyCustomizationPanelWidget::SetupPreviewImage()
{
	if (!Image_Preview || !PreviewMaterialTemplate || !PreviewRenderTarget)
	{
		return;
	}

	PreviewMaterialInstance = UMaterialInstanceDynamic::Create(PreviewMaterialTemplate, this);
	if (!PreviewMaterialInstance)
	{
		return;
	}

	PreviewMaterialInstance->SetTextureParameterValue(PreviewTextureParameterName, PreviewRenderTarget);
	Image_Preview->SetBrushFromMaterial(PreviewMaterialInstance);
}

void ULobbyCustomizationPanelWidget::CapturePreview()
{
	if (PreviewCaptureComponent)
	{
		PreviewCaptureComponent->CaptureScene();
	}
}

void ULobbyCustomizationPanelWidget::FindPreviewActor()
{
	if (PreviewActor)
	{
		return;
	}

	if (!PreviewActorClass)
	{
		return;
	}

	// 현재 로비에는 프리뷰 액터가 하나만 있다고 가정한다.
	// 로비 흐름이 커지면 직접 참조를 주입하는 방식으로 바꿀 수 있다.
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, PreviewActorClass, FoundActors);

	if (!FoundActors.IsEmpty())
	{
		PreviewActor = FoundActors[0];
	}
}

void ULobbyCustomizationPanelWidget::InitializeSelectors()
{
	SetupSelector(Selector_Hair, ECustomizeSlot::Hair);
	SetupSelector(Selector_Top, ECustomizeSlot::Top);
	SetupSelector(Selector_Pants, ECustomizeSlot::Pants);
	SetupSelector(Selector_Socks, ECustomizeSlot::Socks);
	SetupSelector(Selector_Shoes, ECustomizeSlot::Shoes);
	SetupSelector(Selector_Vest, ECustomizeSlot::Vest);
	SetupSelector(Selector_Skirt, ECustomizeSlot::Skirt);
	SetupSelector(Selector_Accessory, ECustomizeSlot::Accessory);
}

void ULobbyCustomizationPanelWidget::SetupSelector(UCustomizePartSelectorWidget* Selector, ECustomizeSlot InSlot)
{
	if (!Selector || !PartCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyCustomizationPanel: Selector 초기화 생략 Slot=%d Selector=%s Catalog=%s"),
			static_cast<int32>(InSlot),
			Selector ? TEXT("Valid") : TEXT("None"),
			PartCatalog ? TEXT("Valid") : TEXT("None"));
		return;
	}

	// 패널은 카탈로그에서 데이터를 가져오고, Selector는 인덱스 이동만 담당한다.
	TArray<UCharacterPartData*> SlotParts;
	PartCatalog->GetPartsBySlot(InSlot, SlotParts);
	UE_LOG(LogTemp, Log, TEXT("LobbyCustomizationPanel: Selector 초기화 Slot=%d Parts=%d"),
		static_cast<int32>(InSlot),
		SlotParts.Num());

	Selector->OnPartChanged.AddDynamic(this, &ULobbyCustomizationPanelWidget::HandlePartChanged);
	Selector->InitializeSelector(InSlot, SlotParts);
}

void ULobbyCustomizationPanelWidget::HandlePartChanged(UCharacterPartData* SelectedPart)
{
	if (!PreviewActor || !SelectedPart)
	{
		return;
	}

	// 프리뷰 액터와 인게임 캐릭터가 같은 커스터마이징 컴포넌트 규약을 공유할 수 있다.
	if (UCharacterCustomizeComponent* CustomizeComponent = PreviewActor->FindComponentByClass<UCharacterCustomizeComponent>())
	{
		CustomizeComponent->ApplyPart(SelectedPart);
		CapturePreview();
	}
}
