#include "SkillCooldownHUDWidget.h"
#include "../UI/SkillCooldownSlotWidget.h"
#include "Blueprint/UserWidget.h"

void USkillCooldownHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SlotWidgets.Reset();

	if (Slot_SkillDecoy)
	{
		SlotWidgets.Add(Slot_SkillDecoy);
	}
}

void USkillCooldownHUDWidget::RegisterSlotWidget(USkillCooldownSlotWidget* InSlotWidget)
{
	if (InSlotWidget)
	{
		SlotWidgets.AddUnique(InSlotWidget);
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
