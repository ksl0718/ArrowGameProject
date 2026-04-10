#include "ResultWidget.h"
#include "Components/TextBlock.h"

void UResultWidget::showResult(bool bIsWin)
{
	
	if (bIsWin)
	{
		if (txt_winLose)
		{
			txt_winLose->SetText(FText::FromString(TEXT("이겼닭 오늘 저녁은 백숙이닭")));
		}
	}else
	{
		if (txt_winLose)
		{
			txt_winLose->SetText(FText::FromString(TEXT("다음에는 잘해보자")));
		}
	}
	
	FTimerHandle showHandle;
	GetWorld()->GetTimerManager().SetTimer(showHandle, [this]() {
		if (this && txt_goLobby)
		{
			txt_goLobby->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Log, TEXT("1초 경과: 로비 가기 버튼 표시"));
		}
	}, 3.0f, false);
}
	

