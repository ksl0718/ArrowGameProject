#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillCooldownHUDWidget.generated.h"

class USkillCooldownSlotWidget;
class UTexture2D;

UCLASS()
class ARROWGAME_API USkillCooldownHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void RegisterSlotWidget(USkillCooldownSlotWidget* InSlotWidget);

	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void UpdateSlotCooldownByIndex(int32 SlotIndex, float RemainingTime, float CooldownDuration);

	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void SetSlotIconByIndex(int32 SlotIndex, UTexture2D* InIconTexture);

	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void SetSlotKeyByIndex(int32 SlotIndex, const FText& InKeyText);

	UFUNCTION(BlueprintPure, Category = "SkillCooldown")
	USkillCooldownSlotWidget* GetSlotWidgetByIndex(int32 SlotIndex) const;

protected:
	
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	USkillCooldownSlotWidget* Slot_SkillDecoy;

private:
	UPROPERTY()
	TArray<USkillCooldownSlotWidget*> SlotWidgets;
};
