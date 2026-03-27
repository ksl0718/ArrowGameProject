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
}