#include "ArrowGameState.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 사용을 위해 필수

void AArrowGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 숫자를 복제하도록 등록
	DOREPLIFETIME(AArrowGameState, bLastThirtySeconds);
	DOREPLIFETIME(AArrowGameState, ListUpdateCount);
}

void AArrowGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	if (HasAuthority()) // 서버에서만 실행
	{
		ListUpdateCount++;
		OnRep_ListUpdateCount(); // 서버 본인 UI 갱신을 위해 직접 호출
	}
}

void AArrowGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	if (HasAuthority())
	{
		ListUpdateCount++;
		OnRep_ListUpdateCount();
	}
}

void AArrowGameState::OnRep_ListUpdateCount()
{
	// 클라이언트가 명단 변경 신호를 받으면 델리게이트 실행
	OnPlayerListChanged.Broadcast();
}