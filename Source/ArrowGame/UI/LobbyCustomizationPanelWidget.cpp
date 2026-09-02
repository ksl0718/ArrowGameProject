#include "LobbyCustomizationPanelWidget.h"

#include "CustomizePartSelectorWidget.h"
#include "ArrowGame/Customization/CharacterCustomizeComponent.h"
#include "ArrowGame/Customization/CharacterPartCatalog.h"
#include "ArrowGame/Customization/CharacterPartData.h"
#include "Kismet/GameplayStatics.h"

void ULobbyCustomizationPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FindPreviewActor();
	InitializeSelectors();
}

void ULobbyCustomizationPanelWidget::FindPreviewActor()
{
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
	SetupSelector(Selector_Vest, ECustomizeSlot::Vest);
	SetupSelector(Selector_Skirt, ECustomizeSlot::Skirt);
	SetupSelector(Selector_Accessory, ECustomizeSlot::Accessory);
}

void ULobbyCustomizationPanelWidget::SetupSelector(UCustomizePartSelectorWidget* Selector, ECustomizeSlot InSlot)
{
	if (!Selector || !PartCatalog)
	{
		return;
	}

	// 패널은 카탈로그에서 데이터를 가져오고, Selector는 인덱스 이동만 담당한다.
	TArray<UCharacterPartData*> SlotParts;
	PartCatalog->GetPartsBySlot(InSlot, SlotParts);

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
	}
}
