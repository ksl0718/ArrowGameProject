#include "ArrowPlayerState.h"
#include "Net/UnrealNetwork.h"

AArrowPlayerState::AArrowPlayerState()
{
	Kills = 0;
	Deaths = 0;
}

// 복제 규칙 등록 (필수!)
void AArrowPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArrowPlayerState, Kills);
	DOREPLIFETIME(AArrowPlayerState, Deaths);
	DOREPLIFETIME(AArrowPlayerState, bIsReady);
	DOREPLIFETIME(AArrowPlayerState, bIsDokkaebi);
}

void AArrowPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
	OnRep_IsReady(); // 서버에서도 UI 갱신을 위해 직접 호출
}

void AArrowPlayerState::OnRep_IsReady()
{
	// 벨을 울린다! (이 신호를 RowWidget이 듣게 됨)
	OnReadyStateChanged.Broadcast(bIsReady);
}