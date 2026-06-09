#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillCooldownHUDWidget.generated.h"

class APawn;
class UHorizontalBox;
class USkillCooldownSlotWidget;
class UTexture2D;

UCLASS()
class ARROWGAME_API USkillCooldownHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SkillCooldown")
	void RebuildSlots(int32 SlotCount);

	UFUNCTION(BlueprintCallable, Category="SkillCooldown")
	void UpdateSlotCooldownByIndex(int32 SlotIndex, float RemainingTime, float CooldownDuration);
	
	UFUNCTION(BlueprintCallable, Category="SkillCooldown")
	void SetSlotIconByIndex(int32 SlotIndex, UTexture2D* InIconTexture);
	
	UFUNCTION(BlueprintCallable, Category="SkillCooldown")
	void SetSlotKeyByIndex(int32 SlotIndex, const FText& InKeyText);

	UFUNCTION(BlueprintPure, Category = "SkillCooldown")
	USkillCooldownSlotWidget* GetSlotWidgetByIndex(int32 SlotIndex) const;

	/** 현재 폰의 ISkillCooldownProvider에서 슬롯 수·아이콘·키를 다시 읽어 HUD를 갱신 */
	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void RefreshFromPawn(APawn* InPawn);

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* HB_SkillSlots;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SkillCooldown")
	TSubclassOf<USkillCooldownSlotWidget> SlotWidgetClass;

private:
	UPROPERTY()
	TArray<USkillCooldownSlotWidget*> SlotWidgets;
};
