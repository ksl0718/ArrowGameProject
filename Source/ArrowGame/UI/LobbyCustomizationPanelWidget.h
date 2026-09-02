#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "LobbyCustomizationPanelWidget.generated.h"

class AActor;
class UCharacterPartCatalog;
class UCharacterPartData;
class UCustomizePartSelectorWidget;

// 로비 커스터마이징 UI 흐름을 담당한다.
// 카탈로그 데이터, 슬롯 선택 위젯, 프리뷰 액터를 연결하는 역할이다.
UCLASS()
class ARROWGAME_API ULobbyCustomizationPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// 선택 가능한 캐릭터 파츠들을 슬롯별로 보관하는 카탈로그 에셋이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<UCharacterPartCatalog> PartCatalog;

	// 로비 레벨에 배치된 프리뷰 액터를 찾을 때 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSubclassOf<AActor> PreviewActorClass;

	// 이 변수명은 WBP_LobbyCustomizationPanel 안의 자식 위젯 이름과 정확히 같아야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Hair;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Top;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Pants;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Vest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Skirt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCustomizePartSelectorWidget> Selector_Accessory;

private:
	UPROPERTY()
	TObjectPtr<AActor> PreviewActor;

	void FindPreviewActor();
	void InitializeSelectors();
	void SetupSelector(UCustomizePartSelectorWidget* Selector, ECustomizeSlot InSlot);

	UFUNCTION()
	void HandlePartChanged(UCharacterPartData* SelectedPart);
};
