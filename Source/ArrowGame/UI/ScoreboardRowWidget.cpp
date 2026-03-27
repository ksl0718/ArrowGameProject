#include "ScoreboardRowWidget.h"
#include "Components/TextBlock.h"
#include "../Core/ArrowPlayerState.h"

void UScoreboardRowWidget::UpdateRow(class AArrowPlayerState* PlayerState)
{
	if (!PlayerState) return;

	// 1. 이름 설정
	if (Txt_Name)
	{
		Txt_Name->SetText(FText::FromString(PlayerState->GetPlayerName()));
	}

	// 2. 킬수 설정 (int32 -> FText 변환)
	if (Txt_Kills)
	{
		Txt_Kills->SetText(FText::AsNumber(PlayerState->GetKills()));
	}

	// 3. 데스수 설정
	if (Txt_Deaths)
	{
		Txt_Deaths->SetText(FText::AsNumber(PlayerState->GetDeaths()));
	}
}