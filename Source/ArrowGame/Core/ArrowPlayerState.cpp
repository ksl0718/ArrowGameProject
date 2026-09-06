#include "ArrowPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ArrowGame/ArrowGameInstance.h"
#include "ArrowGameState.h"

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
	DOREPLIFETIME(AArrowPlayerState, SelectedCustomizePreset);
}

void AArrowPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AArrowPlayerState* NewPlayerState = Cast<AArrowPlayerState>(PlayerState);
	if (!NewPlayerState)
	{
		return;
	}

	// SeamlessTravel은 PlayerState를 유지하거나 새 인스턴스로 복사할 수 있다.
	// 엔진 기본 값만 복사되면 커마 프리셋이 비어서 전투맵 캐릭터가 기본 외형으로 돌아간다.
	NewPlayerState->SelectedCustomizePreset = SelectedCustomizePreset;
	NewPlayerState->bIsReady = bIsReady;
	NewPlayerState->bIsDokkaebi = bIsDokkaebi;
	UE_LOG(LogTemp, Log, TEXT("PlayerState CopyProperties: 커마 프리셋 %d개 복사"), SelectedCustomizePreset.SelectedParts.Num());
}

void AArrowPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	AArrowPlayerState* OldPlayerState = Cast<AArrowPlayerState>(PlayerState);
	if (!OldPlayerState)
	{
		return;
	}

	// 기존 PlayerState에서 현재 PlayerState로 값을 복원하는 경로도 같이 처리한다.
	SelectedCustomizePreset = OldPlayerState->SelectedCustomizePreset;
	bIsReady = OldPlayerState->bIsReady;
	bIsDokkaebi = OldPlayerState->bIsDokkaebi;
	UE_LOG(LogTemp, Log, TEXT("PlayerState OverrideWith: 커마 프리셋 %d개 복원"), SelectedCustomizePreset.SelectedParts.Num());
}

void AArrowPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
	OnRep_IsReady(); // 서버에서도 UI 갱신을 위해 직접 호출
}

void AArrowPlayerState::SetIsDokkaebi(bool bNewState)
{
	if (!HasAuthority())
	{
		return;
	}
	bIsDokkaebi = bNewState;
	ForceNetUpdate();
}

void AArrowPlayerState::ServerSetCustomizePreset_Implementation(const FCharacterCustomizePreset& NewPreset)
{
	SetCustomizePresetOnServer(NewPreset);
}

void AArrowPlayerState::SetCustomizePresetOnServer(const FCharacterCustomizePreset& NewPreset)
{
	if (!HasAuthority())
	{
		return;
	}

	SelectedCustomizePreset = NewPreset;
	UE_LOG(LogTemp, Log, TEXT("PlayerState: 커마 프리셋 저장 - %d개 파츠"), SelectedCustomizePreset.SelectedParts.Num());

	if (UArrowGameInstance* ArrowGI = GetWorld() ? Cast<UArrowGameInstance>(GetWorld()->GetGameInstance()) : nullptr)
	{
		ArrowGI->CacheCustomizePresetForPlayer(this, SelectedCustomizePreset);
	}

	// Listen Server의 호스트 화면은 OnRep이 호출되지 않으므로 직접 알려준다.
	OnRep_SelectedCustomizePreset();
	ForceNetUpdate();
}

void AArrowPlayerState::OnRep_IsReady()
{
	OnReadyStateChanged.Broadcast(bIsReady);

	if (AArrowGameState* GS = GetWorld()->GetGameState<AArrowGameState>())
	{
		GS->OnPlayerListChanged.Broadcast();
	}
}

void AArrowPlayerState::OnRep_SelectedCustomizePreset()
{
	OnCustomizePresetChanged.Broadcast(SelectedCustomizePreset);
}
