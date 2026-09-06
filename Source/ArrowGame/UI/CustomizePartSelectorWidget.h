#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "CustomizePartSelectorWidget.generated.h"

class UButton;
class UCharacterPartData;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomizePartChanged, UCharacterPartData*, SelectedPart);

// 커스터마이징 슬롯 하나를 담당하는 재사용 선택 위젯.
// 이 위젯은 선택 상태만 관리하고, 선택된 파츠를 어떻게 적용할지는 상위 패널이 결정한다.
UCLASS()
class ARROWGAME_API UCustomizePartSelectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 상위 패널이 카탈로그에서 슬롯별 파츠 목록을 가져온 뒤 호출한다.
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void InitializeSelector(ECustomizeSlot InSlot, const TArray<UCharacterPartData*>& InParts);

	// 현재 선택 인덱스가 바뀔 때마다 호출된다. 초기 선택 적용 시에도 호출된다.
	UPROPERTY(BlueprintAssignable, Category = "Customization")
	FOnCustomizePartChanged OnPartChanged;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 아래 변수명은 WBP_CustomizePartSelector 안의 위젯 이름과 정확히 같아야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Prev;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Next;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SlotName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Index;

private:
	UPROPERTY()
	TArray<TObjectPtr<UCharacterPartData>> Parts;

	UPROPERTY()
	ECustomizeSlot CustomizeSlot = ECustomizeSlot::Hair;

	int32 CurrentIndex = 0;

	UFUNCTION()
	void SelectPreviousPart();

	UFUNCTION()
	void SelectNextPart();

	void ApplyCurrentPart();
	void UpdateTexts();
	FText GetSlotDisplayText() const;
};
