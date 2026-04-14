#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillCooldownSlotWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

UCLASS()
class ARROWGAME_API USkillCooldownSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void UpdateCooldown(float RemainingTime, float CooldownDuration);

	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void SetSkillIcon(UTexture2D* InIconTexture);

	UFUNCTION(BlueprintCallable, Category = "SkillCooldown")
	void SetSkillKeyText(const FText& InKeyText);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Cooldown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Remaining;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_SkillKey;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Icon;
};
