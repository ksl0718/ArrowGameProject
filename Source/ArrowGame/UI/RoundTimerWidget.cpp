#include "RoundTimerWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void URoundTimerWidget::StartTimer(float Duration)
{
	RemainingTime = Duration;;
	
	GetWorld()->GetTimerManager().SetTimer(UITimerHandle, this, &URoundTimerWidget::UpdateTimeText,
		1.0f, true, 0.0f);
}

void URoundTimerWidget::UpdateTimeText()
{
	if (RemainingTime < 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(UITimerHandle);
		RemainingTime = 0.0f;
	}
	
	int32 TotalSeconds = FMath::CeilToInt(RemainingTime);
	int32 Minutes = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;
	
	if (txt_Time)
	{
		FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		txt_Time->SetText(FText::FromString(TimeString));
	}
	
	RemainingTime -= 1.0f;
}