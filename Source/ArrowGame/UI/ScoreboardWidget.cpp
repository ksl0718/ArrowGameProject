#include "ScoreboardWidget.h"
#include "Components/VerticalBox.h"
#include "ScoreboardRowWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "../Core/ArrowPlayerState.h" 
#include "GameFramework/GameState.h"

void UScoreboardWidget::RefreshScoreboard()
{
	if (!VB_PlayerList || !RowWidgetClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("RowWidgetClass or VB is Missing!"));
		return;
	}

	VB_PlayerList->ClearChildren();

	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Count: %d"), GameState->PlayerArray.Num());
        
		for (APlayerState* PS : GameState->PlayerArray)
		{
			// 1. 위젯을 메모리에 찍어낸다
			AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS);
			
			
	        if (ArrowPS)
	        {
	        	UScoreboardRowWidget* NewRow = CreateWidget<UScoreboardRowWidget>(this, RowWidgetClass);
		        if (NewRow)
		        {
	        		NewRow->UpdateRow(ArrowPS);

	        		// ★ 바로 이 녀석이 "Add Child to Vertical Box" 노드의 C++ 버전입니다!
	        		VB_PlayerList->AddChildToVerticalBox(NewRow); 
	            
	        		UE_LOG(LogTemp, Warning, TEXT("Added Row for Player: %s"), *PS->GetPlayerName());
		        }
	        }
		}
	}
}
