#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "LobbyCustomizationPanelWidget.generated.h"

class AActor;
class UCharacterPartCatalog;
class UCharacterPartData;
class UCustomizePartSelectorWidget;
class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

// 로비 커스터마이징 UI 흐름을 담당한다.
// 카탈로그 데이터, 슬롯 선택 위젯, 프리뷰 액터를 연결하는 역할이다.
UCLASS()
class ARROWGAME_API ULobbyCustomizationPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// BP에서 로컬로 스폰한 프리뷰 액터를 패널에 넘길 때 사용한다.
	// 리슨 서버 PIE처럼 화면이 여러 개일 때 각 화면이 자기 프리뷰만 조작하게 만드는 주입 지점이다.
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetPreviewActor(AActor* InPreviewActor);

	// 로비 프리뷰 캐릭터에 현재 적용된 커마 프리셋을 가져온다.
	// 저장 위치를 직접 바꾸지 않고, 시작/준비 버튼을 가진 로비 보드가 필요한 시점에 가져가게 하기 위한 함수다.
	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool GetCurrentPreviewPreset(FCharacterCustomizePreset& OutPreset) const;

	// 현재 프리뷰 프리셋을 내 PlayerState에 확정 저장한다.
	// 클라이언트가 호출하면 내부에서 PlayerState의 Server RPC로 서버에 전달된다.
	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool CommitCurrentPresetToPlayerState() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 선택 가능한 캐릭터 파츠들을 슬롯별로 보관하는 카탈로그 에셋이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<UCharacterPartCatalog> PartCatalog;

	// 로비 레벨에 배치된 프리뷰 액터를 찾을 때 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSubclassOf<AActor> PreviewActorClass;

	// 로컬 프리뷰 액터의 스폰 위치다. SceneCapture2D가 바라보는 지점에 맞춘다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	FTransform PreviewActorSpawnTransform = FTransform::Identity;

	// BP에서 로컬 SceneCapture 액터를 스폰할 때 사용할 클래스다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	TSubclassOf<AActor> PreviewCaptureActorClass;

	// 로컬 SceneCapture 액터의 스폰 위치다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	FTransform PreviewCaptureSpawnTransform = FTransform::Identity;

	// 동적 RenderTarget을 표시할 UI 머티리얼 템플릿이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	TObjectPtr<UMaterialInterface> PreviewMaterialTemplate;

	// UI 머티리얼 안의 Texture Parameter 이름이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	FName PreviewTextureParameterName = TEXT("PreviewTexture");

	// BP에서 생성할 로컬 RenderTarget의 기본 크기다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Preview Render")
	int32 PreviewRenderTargetSize = 512;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Preview;

	// 이 변수명은 WBP_LobbyCustomizationPanel 안의 자식 위젯 이름과 정확히 같아야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Hair;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Top;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Pants;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Socks;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Shoes;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Vest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Skirt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Accessory;

private:
	UPROPERTY()
	TObjectPtr<AActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<AActor> PreviewCaptureActor;

	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> PreviewCaptureComponent;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> PreviewRenderTarget;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterialInstance;

	bool bSpawnedPreviewActor = false;
	bool bSpawnedPreviewCaptureActor = false;

	void SetupLocalPreview();
	void CreateLocalPreviewRenderTarget();
	void SpawnLocalPreviewActor();
	void SpawnLocalPreviewCaptureActor();
	void SetupPreviewImage();
	void CapturePreview();
	void FindPreviewActor();
	void InitializeSelectors();
	void SetupSelector(UCustomizePartSelectorWidget* Selector, ECustomizeSlot InSlot);

	UFUNCTION()
	void HandlePartChanged(UCharacterPartData* SelectedPart);
};
