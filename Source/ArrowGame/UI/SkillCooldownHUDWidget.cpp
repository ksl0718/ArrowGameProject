#include "SkillCooldownHUDWidget.h"
#include "SkillCooldownSlotWidget.h"
#include "../Character/SkillCooldownProvider.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void USkillCooldownHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SlotWidgets.Reset();
	if (HB_SkillSlots)
	{
		HB_SkillSlots->ClearChildren();
	}

	// 위젯이 Construct된 뒤 폰 메타(아이콘·키)를 다시 적용 (SetPawn보다 늦게 Construct되는 경우 대비)
	if (APlayerController* PC = GetOwningPlayer())
	{
		RefreshFromPawn(PC->GetPawn());
	}
}


void USkillCooldownHUDWidget::RebuildSlots(int32 SlotCount)
{
	SlotWidgets.Reset();
	
	if (!HB_SkillSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillCooldownHUDWidget: HB_SkillSlots 바인딩 없음 — WBP HUD에 HorizontalBox 이름을 'HB_SkillSlots'로 맞추세요."));
		return;
	}

	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillCooldownHUDWidget: SlotWidgetClass 미지정 — WBP HUD Class Defaults에서 슬롯 위젯 클래스를 지정하세요."));
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

void USkillCooldownHUDWidget::RefreshFromPawn(APawn* InPawn)
{
	const ISkillCooldownProvider* Provider = Cast<ISkillCooldownProvider>(InPawn);
	if (!Provider)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillHUD Refresh: Pawn=%s — ISkillCooldownProvider 아님"), *GetNameSafe(InPawn));
		RebuildSlots(0);
		return;
	}

	const int32 SlotCount = FMath::Max(0, Provider->GetSkillSlotCount());
	UE_LOG(LogTemp, Warning, TEXT("SkillHUD Refresh: Pawn=%s SlotCount=%d"), *GetNameSafe(InPawn), SlotCount);
	RebuildSlots(SlotCount);

	for (int32 i = 0; i < SlotCount; ++i)
	{
		UTexture2D* Icon = nullptr;
		FText KeyText = FText::GetEmpty();

		if (Provider->GetSkillHudMetaByIndex(i, Icon, KeyText))
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillHUD slot %d: Icon=%s Key='%s'"),
				i,
				Icon ? *Icon->GetName() : TEXT("NULL"),
				*KeyText.ToString());

			SetSlotIconByIndex(i, Icon);
			SetSlotKeyByIndex(i, KeyText);
		}
	}
}
