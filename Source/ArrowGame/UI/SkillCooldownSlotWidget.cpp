#include "SkillCooldownSlotWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

void USkillCooldownSlotWidget::UpdateCooldown(float RemainingTime, float CooldownDuration)
{
	const float SafeDuration = FMath::Max(0.01f, CooldownDuration);
	const float ClampedRemaining = FMath::Max(0.0f, RemainingTime);
	const float Percent = FMath::Clamp(ClampedRemaining / SafeDuration, 0.0f, 1.0f);

	if (PB_Cooldown)
	{
		PB_Cooldown->SetPercent(Percent);
	}

	if (Txt_Remaining)
	{
		if (ClampedRemaining > 0.0f)
		{
			const int32 DisplaySeconds = FMath::CeilToInt(ClampedRemaining);
			Txt_Remaining->SetText(FText::AsNumber(DisplaySeconds));
			Txt_Remaining->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Txt_Remaining->SetText(FText::GetEmpty());
			Txt_Remaining->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void USkillCooldownSlotWidget::SetSkillIcon(UTexture2D* InIconTexture)
{
	if (!Img_Icon)
	{
		return;
	}

	if (!InIconTexture)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(InIconTexture);
	const float IconSize = 48.f;
	IconBrush.ImageSize = FVector2D(IconSize, IconSize);
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	Img_Icon->SetBrush(IconBrush);
	Img_Icon->SetColorAndOpacity(FLinearColor::White);
	Img_Icon->SetRenderOpacity(1.f);
	Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void USkillCooldownSlotWidget::SetSkillKeyText(const FText& InKeyText)
{
	if (Txt_SkillKey)
	{
		Txt_SkillKey->SetText(InKeyText);
	}
}
