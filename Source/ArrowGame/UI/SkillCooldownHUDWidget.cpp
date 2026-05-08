#include "SkillCooldownHUDWidget.h"
#include "SkillCooldownSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void USkillCooldownHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SlotWidgets.Reset();
	if (HB_SkillSlots)
	{
		HB_SkillSlots->ClearChildren();
	}
}


void USkillCooldownHUDWidget::RebuildSlots(int32 SlotCount)
{
	SlotWidgets.Reset();
	
	if (!HB_SkillSlots || !SlotWidgetClass)
	{
		return;
	}
	
	HB_SkillSlots->ClearChildren();
	
	const int32 SafeCount = FMath::Max(0, SlotCount);
	for (int32 i = 0; i < SafeCount; ++i)
	{
		USkillCooldownSlotWidget* NewSlot =
			CreateWidget<USkillCooldownSlotWidget>(GetWorld(), SlotWidgetClass);
		if (!NewSlot)
		{
			continue;
		}
		if (UHorizontalBoxSlot* AddedSlot = HB_SkillSlots->AddChildToHorizontalBox(NewSlot))
		{
			AddedSlot->SetPadding(FMargin(4.f, 0.f));
		}
		SlotWidgets.Add(NewSlot);
	}
}


void USkillCooldownHUDWidget::UpdateSlotCooldownByIndex(int32 SlotIndex, float RemainingTime, float CooldownDuration)
{
	if (USkillCooldownSlotWidget* CooldownSlot = GetSlotWidgetByIndex(SlotIndex))
	{
		CooldownSlot->UpdateCooldown(RemainingTime, CooldownDuration);
	}
}

void USkillCooldownHUDWidget::SetSlotIconByIndex(int32 SlotIndex, UTexture2D* InIconTexture)
{
	if (USkillCooldownSlotWidget* CooldownSlot = GetSlotWidgetByIndex(SlotIndex))
	{
		CooldownSlot->SetSkillIcon(InIconTexture);
	}
}

void USkillCooldownHUDWidget::SetSlotKeyByIndex(int32 SlotIndex, const FText& InKeyText)
{
	if (USkillCooldownSlotWidget* CooldownSlot = GetSlotWidgetByIndex(SlotIndex))
	{
		CooldownSlot->SetSkillKeyText(InKeyText);
	}
}

USkillCooldownSlotWidget* USkillCooldownHUDWidget::GetSlotWidgetByIndex(int32 SlotIndex) const
{
	return SlotWidgets.IsValidIndex(SlotIndex) ? SlotWidgets[SlotIndex] : nullptr;
}
