#include "LobbyRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Core/ArrowPlayerState.h"

void ULobbyRowWidget::Setup(class AArrowPlayerState* InPS)
{
	if (InPS)
	{
		OwningPlayerState = InPS; // 약포인터 저장
		
		txt_PlayerName->SetText(FText::FromString(InPS->GetPlayerName()));
		
		UpdateReadyVisual(InPS->IsReady());
		
		InPS->OnReadyStateChanged.AddDynamic(this, &ULobbyRowWidget::UpdateReadyVisual);
	}
}

void ULobbyRowWidget::UpdateReadyVisual(bool bNewReady)
{
	if (ReadyStateImg)
	{
		// 1. 강제로 크기를 고정 (이미지가 너무 작아진 경우 대비)
		ReadyStateImg->SetDesiredSizeOverride(FVector2D(100.f, 100.f));

		// 2. 투명도를 강제로 1.0으로 설정 (어디선가 Alpha가 0이 된 경우 대비)
		ReadyStateImg->SetRenderOpacity(1.0f);

		// 기존 로직
		ReadyStateImg->SetVisibility(bNewReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        
		UE_LOG(LogTemp, Warning, TEXT("Visibility Set to: %s"), bNewReady ? TEXT("Visible") : TEXT("Hidden"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ReadyStateImg is NULL!"));
	}
}