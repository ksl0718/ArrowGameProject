#include "CountdownWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UCountdownWidget::StartCountdown(int32 StartValue)
{
	CurrentCount = StartValue;
	UpdateCount();

	// 1초마다 UpdateCount 함수를 반복 호출
	GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UCountdownWidget::UpdateCount, 1.0f, true);
}

void UCountdownWidget::UpdateCount()
{
	if (txt_Count == nullptr) UE_LOG(LogTemp, Error, TEXT("txt_Count 연결 실패!"));
	if (CountdownAnimation == nullptr) UE_LOG(LogTemp, Error, TEXT("Anim_Tick 애니메이션 연결 실패!"));
	
	if (CurrentCount >= 0)
	{
		// 1. 텍스트 업데이트
		if (txt_Count)
		{
			// 0초일 때는 "GO!" 혹은 "START!" 표시 가능
			FString DisplayText = (CurrentCount == 0) ? TEXT("START!") : FString::FromInt(CurrentCount);
			txt_Count->SetText(FText::FromString(DisplayText));
			
			UE_LOG(LogTemp, Warning, TEXT("현재 숫자: %d, 애니메이션 재생 시도!"), CurrentCount);
		}

		// 2. 애니메이션 재생 (블루프린트에서 만든 그 애니메이션!)
		if (CountdownAnimation)
		{
			PlayAnimation(CountdownAnimation);
		}

		CurrentCount--;
	}
	else
	{
		// 3. 카운트다운 종료 시 타이머 해제 및 위젯 제거
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		RemoveFromParent();
	}
}