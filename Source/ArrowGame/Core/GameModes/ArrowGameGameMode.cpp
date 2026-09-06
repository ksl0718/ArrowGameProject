// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGameGameMode.h"
#include "ArrowGame/Core/ArrowGamePlayerController.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "ArrowGame/Core/ArrowGameState.h"
#include "ArrowGame//ArrowGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AArrowGameGameMode::AArrowGameGameMode()
{
	bUseSeamlessTravel = true;
	PlayerStateClass = AArrowPlayerState::StaticClass();

	// 트래블·역할 배정 전까지 폰 없음 → RestartPlayer 한 번에 빙의·입력·BeginPlay 순서 보장
	bStartPlayersAsSpectators = true;
}

void AArrowGameGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	UE_LOG(LogTemp, Warning, TEXT("심리스 트래블 완료 — 전원 접속 대기 후 매치 시작 검사"));
	SyncPendingPlayersFromGameState();
	ScheduleTryStartMatch(0.5f);
}

bool AArrowGameGameMode::IsBlockedByTravel() const
{
	const UWorld* World = GetWorld();
	return World && World->IsInSeamlessTravel();
}

void AArrowGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	bBattleInputsUnlocked = false;

	// 심리스 트래블: 플레이어 PC·PlayerState가 순차 도착하므로 즉시 시작하지 않고 재시도
	SyncPendingPlayersFromGameState();
	ScheduleTryStartMatch(1.0f);
}

void AArrowGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	RegisterPendingPlayer(NewPlayer);
	ScheduleTryStartMatch(0.5f);
}

void AArrowGameGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		return;
	}

	RegisterPendingPlayer(NewPlayer);

	if (bGameStarted)
	{
		CatchUpPlayerAfterMatchStarted(NewPlayer);
		return;
	}

	// 역할 배정 전 기본 폰 스폰 방지 (HumanClass + IsDokkaebi=false 조합이 꼬이는 원인)
	ScheduleTryStartMatch(0.5f);
}

void AArrowGameGameMode::Logout(AController* Exiting)
{
	if (APlayerController* ExitingPC = Cast<APlayerController>(Exiting))
	{
		PendingPlayers.Remove(ExitingPC);
	}
	Super::Logout(Exiting);
}

void AArrowGameGameMode::RegisterPendingPlayer(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		return;
	}

	RestoreCustomizePresetIfNeeded(NewPlayer->GetPlayerState<AArrowPlayerState>());
	PendingPlayers.AddUnique(NewPlayer);
}

void AArrowGameGameMode::SyncPendingPlayersFromGameState()
{
	AGameStateBase* GS = GameState;
	if (!GS)
	{
		return;
	}

	TArray<APlayerController*> Synced;
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS))
		{
			RestoreCustomizePresetIfNeeded(ArrowPS);

			if (APlayerController* PC = ArrowPS->GetPlayerController())
			{
				if (IsValid(PC))
				{
					Synced.AddUnique(PC);
				}
			}
		}
	}

	if (Synced.Num() > 0)
	{
		PendingPlayers = MoveTemp(Synced);
	}

	PendingPlayers.RemoveAll([](const APlayerController* PC)
	{
		return !IsValid(PC);
	});
}

void AArrowGameGameMode::RestoreCustomizePresetIfNeeded(AArrowPlayerState* PlayerState) const
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	if (PlayerState->GetCustomizePreset().SelectedParts.Num() > 0)
	{
		return;
	}

	UArrowGameInstance* ArrowGI = GetWorld() ? Cast<UArrowGameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	if (!ArrowGI)
	{
		return;
	}

	FCharacterCustomizePreset CachedPreset;
	if (!ArrowGI->TryGetCachedCustomizePresetForPlayer(PlayerState, CachedPreset))
	{
		return;
	}

	// 심리스 트래블 복사가 비었거나 새 PlayerState가 생긴 경우, 서버 PlayerState를 캐시 값으로 복원한다.
	PlayerState->SetCustomizePresetOnServer(CachedPreset);
	UE_LOG(LogTemp, Log, TEXT("ArrowGameGameMode: 커마 프리셋 캐시 복원 - %s / %d개"),
		*PlayerState->GetPlayerName(),
		CachedPreset.SelectedParts.Num());
}

int32 AArrowGameGameMode::GetRequiredPlayersToStart() const
{
	int32 RequiredToStart = FMath::Max(ExpectedPlayers, 2);

	if (const UWorld* World = GetWorld())
	{
		if (const UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(World->GetGameInstance()))
		{
			const int32 FromLobby = ArrowGI->GetMatchStartPlayerCount();
			if (FromLobby > 0)
			{
				RequiredToStart = FromLobby;
			}
		}
	}

	// 로비 MatchStartPlayerCount가 0이어도 심리스로 PlayerArray만 4명인 경우 조기(2명) 시작 방지
	if (const AGameStateBase* GS = GameState)
	{
		const int32 GSCount = GS->PlayerArray.Num();
		if (GSCount > RequiredToStart)
		{
			RequiredToStart = GSCount;
		}
	}

	return RequiredToStart;
}

bool AArrowGameGameMode::AreAllPendingPlayersReadyForAssign() const
{
	if (PendingPlayers.Num() == 0)
	{
		return false;
	}

	for (const APlayerController* PC : PendingPlayers)
	{
		if (!IsValid(PC))
		{
			return false;
		}
		if (!PC->GetPlayerState<AArrowPlayerState>())
		{
			return false;
		}
	}
	return true;
}

void AArrowGameGameMode::ScheduleTryStartMatch(float DelaySeconds)
{
	if (bGameStarted || !HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(MatchStartRetryTimerHandle);
	World->GetTimerManager().SetTimer(
		MatchStartRetryTimerHandle,
		this,
		&AArrowGameGameMode::TryStartMatchIfReady,
		DelaySeconds,
		false);
}

void AArrowGameGameMode::TryStartMatchIfReady()
{
	if (bGameStarted)
	{
		return;
	}

	if (IsBlockedByTravel())
	{
		UE_LOG(LogTemp, Warning, TEXT("매치 시작 대기: 심리스 트래블 진행 중"));
		ScheduleTryStartMatch(0.5f);
		return;
	}

	SyncPendingPlayersFromGameState();

	const int32 RequiredToStart = GetRequiredPlayersToStart();
	const int32 GameStatePlayerCount = GameState ? GameState->PlayerArray.Num() : 0;

	UE_LOG(LogTemp, Warning,
		TEXT("전투 맵 시작 체크 | Pending(PC): %d | GameState: %d | 요구: %d | Travel=%s"),
		PendingPlayers.Num(), GameStatePlayerCount, RequiredToStart,
		IsBlockedByTravel() ? TEXT("InProgress") : TEXT("Done"));

	// 로비에서 나온 인원 전원: PlayerArray == Controller 연결 수 == 요구 인원 (호스트만 먼저 도착 시 시작 금지)
	const bool bEveryonePresent =
		GameStatePlayerCount == RequiredToStart
		&& PendingPlayers.Num() == RequiredToStart
		&& PendingPlayers.Num() == GameStatePlayerCount;

	if (bEveryonePresent && AreAllPendingPlayersReadyForAssign())
	{
		UE_LOG(LogTemp, Warning, TEXT("조건 충족! 게임 시작 시퀀스 실행"));
		AssignDokkaebiAndStart();
	}
	else
	{
		ScheduleTryStartMatch(0.5f);
	}
}

void AArrowGameGameMode::CatchUpPlayerAfterMatchStarted(APlayerController* PC)
{
	if (!IsValid(PC))
	{
		return;
	}

	// 호스트 등 이미 Assign에서 스폰된 PC를 다시 RestartPlayer 하면 도깨비→궁수/입력 잠김이 납니다.
	if (ControllersSpawnedAtMatchStart.Contains(PC))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("CatchUp 생략 (이미 매치 시작 스폰 완료): %s"), *PC->GetName());
		return;
	}

	UE_LOG(LogTemp, Error,
		TEXT("매치 시작 이후 플레이어 도착 — 역할/스폰 동기화: %s (트래블 지연)"),
		*PC->GetName());

	SpawnPlayerForMatchStart(PC);

	if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
	{
		MyPC->SetPlayerEnabledState(bBattleInputsUnlocked);
	}
}

void AArrowGameGameMode::SpawnPlayerForMatchStart(APlayerController* PC)
{
	if (!IsValid(PC))
	{
		return;
	}

	// CatchUp/HandleStartingNewPlayer 재진입 방지 (지연 스폰 중에도 동일 PC 중복 RestartPlayer 금지)
	ControllersSpawnedAtMatchStart.Add(PC);

	APawn* ExistingPawn = PC->GetPawn();
	if (ExistingPawn)
	{
		// 심리스 트래블 직후 로비 DefaultPawn을 같은 프레임에 Destroy+Restart 하면
		// 파괴된 컴포넌트가 EndOfFrame 업데이트 큐에 남아 LevelTick assertion이 날 수 있음.
		PC->UnPossess();
		ExistingPawn->Destroy();
		PC->StartSpot = nullptr;

		TWeakObjectPtr<APlayerController> WeakPC(PC);
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakPC]()
		{
			if (APlayerController* P = WeakPC.Get())
			{
				RestartPlayer(P);
			}
		}));
		return;
	}

	PC->StartSpot = nullptr;
	RestartPlayer(PC);
}

void AArrowGameGameMode::AssignDokkaebiAndStart()
{
	SyncPendingPlayersFromGameState();

	const int32 RequiredToStart = GetRequiredPlayersToStart();
	if (PendingPlayers.Num() < RequiredToStart || !AreAllPendingPlayersReadyForAssign())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("AssignDokkaebiAndStart 보류 | Pending: %d | 요구: %d | PlayerState 준비: %s"),
			PendingPlayers.Num(), RequiredToStart,
			AreAllPendingPlayersReadyForAssign() ? TEXT("Yes") : TEXT("No"));
		ScheduleTryStartMatch(0.5f);
		return;
	}

	ControllersSpawnedAtMatchStart.Empty();

	for (APlayerController* PC : PendingPlayers)
	{
		if (AArrowPlayerState* PS = PC->GetPlayerState<AArrowPlayerState>())
		{
			PS->SetIsDokkaebi(false);
		}
	}

	if (!DokkaebiClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignDokkaebiAndStart: DokkaebiClass가 비어 있습니다. BP_ArrowGameGameMode를 확인하세요."));
	}
	if (!HumanClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignDokkaebiAndStart: HumanClass가 비어 있습니다. BP_ArrowGameGameMode를 확인하세요."));
	}

	// 인원 1명일 때 RandRange(0,0) → 항상 0번(보통 호스트)만 도깨비 되는 문제 방지: 반드시 RequiredToStart명일 때만 여기 도달
	APlayerController* DokkaebiPC = PendingPlayers[FMath::RandRange(0, PendingPlayers.Num() - 1)];
	check(DokkaebiPC);

	int32 DokkaebiCount = 0;
	for (APlayerController* PC : PendingPlayers)
	{
		AArrowPlayerState* PS = PC->GetPlayerState<AArrowPlayerState>();
		if (!PS)
		{
			UE_LOG(LogTemp, Error, TEXT("AssignDokkaebiAndStart: %s PlayerState 없음 — 스킵"), *GetNameSafe(PC));
			continue;
		}

		const bool bIsDokkaebi = (PC == DokkaebiPC);
		PS->SetIsDokkaebi(bIsDokkaebi);
		if (bIsDokkaebi)
		{
			++DokkaebiCount;
		}

		UE_LOG(LogTemp, Warning, TEXT("역할 배정 | %s | %s | PawnClass=%s"),
			*PS->GetPlayerName(),
			bIsDokkaebi ? TEXT("Dokkaebi") : TEXT("Archer"),
			bIsDokkaebi ? *GetNameSafe(DokkaebiClass) : *GetNameSafe(HumanClass));

		SpawnPlayerForMatchStart(PC);

		if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
		{
			MyPC->SetPlayerEnabledState(false);
		}
	}

	if (DokkaebiCount != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("도깨비 인원 오류: %d명 (정확히 1명이어야 함)"), DokkaebiCount);
	}

	bGameStarted = true;

	if (UWorld* World = GetWorld())
	{
		if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(World->GetGameInstance()))
		{
			ArrowGI->SetMatchStartPlayerCount(0);
		}
	}
	const TArray<APlayerController*> PlayersForThisMatch = PendingPlayers;

	FTimerHandle UIStartHandle;
	GetWorldTimerManager().SetTimer(UIStartHandle, [this, PlayersForThisMatch]()
	{
		for (APlayerController* PC : PlayersForThisMatch)
		{
			if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
			{
				MyPC->Client_StartCountdown(5.0f);
			}
		}

		GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AArrowGameGameMode::ActualStartGame, 5.0f, false);
	}, 1.0f, false);
}

AActor* AArrowGameGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> AllStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AllStarts);

	if (AArrowPlayerState* PS = Player->GetPlayerState<AArrowPlayerState>())
	{
		if (PS->IsDokkaebi())
		{
			for (AActor* Start : AllStarts)
			{
				if (Start->ActorHasTag(FName("Dokkaebi")))
				{
					return Start;
				}
			}
			UE_LOG(LogTemp, Error,
				TEXT("PlayerStart에 'Dokkaebi' 태그가 없습니다! %s — 궁수 스폰으로 떨어지지 않도록 기본 ChoosePlayerStart 사용"),
				*PS->GetPlayerName());
			return Super::ChoosePlayerStart_Implementation(Player);
		}
		else
		{
			TArray<AActor*> ArcherStarts;
			for (AActor* Start : AllStarts)
			{
				if (!Start->ActorHasTag(FName("Dokkaebi")))
					ArcherStarts.Add(Start);
			}
			if (ArcherStarts.Num() > 0)
				return ArcherStarts[FMath::RandRange(0, ArcherStarts.Num() - 1)];
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

UClass* AArrowGameGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (AArrowPlayerState* PS = InController->GetPlayerState<AArrowPlayerState>())
	{
		// 로그 추가: 이 플레이어가 도깨비인지 서버가 어떻게 판단하는지 출력
		UE_LOG(LogTemp, Warning, TEXT("플레이어 %s 도깨비 여부: %s"), 
			   *PS->GetPlayerName(), PS->IsDokkaebi() ? TEXT("True") : TEXT("False"));

		if (PS->IsDokkaebi()) 
		{
			return DokkaebiClass; //
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState를 찾을 수 없습니다!"));
	}

	return HumanClass; //
}

void AArrowGameGameMode::ActualStartGame()
{
	bRoundEnded = false;
	bBattleInputsUnlocked = true;

	SyncPendingPlayersFromGameState();

	auto EnablePlayerForBattle = [this](APlayerController* PC)
	{
		if (!IsValid(PC))
		{
			return;
		}

		if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
		{
			MyPC->SetPlayerEnabledState(true);
			MyPC->Client_BattleStart(RoundTimeLimit);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ActualStartGame: ArrowGamePlayerController 아님 — %s"), *GetNameSafe(PC));
		}
	};

	if (AGameStateBase* GS = GameState)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS))
			{
				EnablePlayerForBattle(ArrowPS->GetPlayerController());
			}
		}
	}
	else
	{
		for (APlayerController* PC : PendingPlayers)
		{
			EnablePlayerForBattle(PC);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("라운드 타이머 시작! 제한 시간: %f초"), RoundTimeLimit);
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AArrowGameGameMode::OnRoundTimeUp, RoundTimeLimit, false);

	const float TimeUntilLastThirty = RoundTimeLimit - 30.0f;
	if (TimeUntilLastThirty > 0.0f)
	{
		GetWorldTimerManager().SetTimer(LastThirtySecondsTimerHandle, this, &AArrowGameGameMode::OnLastThirtySecondsStart, TimeUntilLastThirty, false);
	}
	else
	{
		OnLastThirtySecondsStart();
	}
}


void AArrowGameGameMode::ActorDied(AActor* DeadActor, AController* KillerController)
{
	if (bRoundEnded) return;
	


	APawn* DeadPawn = Cast<APawn>(DeadActor);
	if(!DeadPawn) return;

	if (KillerController && KillerController != DeadPawn->GetController())
	{
		AArrowPlayerState* KillerPS = KillerController->GetPlayerState<AArrowPlayerState>();
		if (KillerPS)
		{
			KillerPS->AddKill(); // 또는 AddKill() 함수 호출
			UE_LOG(LogTemp, Warning, TEXT("Killer: %s, Kills: %d"), *KillerPS->GetPlayerName(), KillerPS->GetKills());
		}
	}
	
	
	// 1. 죽은 액터가 플레이어인지 확인

	AArrowPlayerState* VictimPS = DeadPawn->GetPlayerState<AArrowPlayerState>();
	if (VictimPS)
	{
		VictimPS->AddDeath();
		UE_LOG(LogTemp, Warning, TEXT("deadpawn is dokkaebi: %d"),VictimPS->IsDokkaebi());
		
		if (VictimPS->IsDokkaebi())
		{
			EndRound(false); // Humans Win
			return;          // 도깨비는 리스폰 없음
		}
	}

	if (DeadPawn->IsPlayerControlled())
	{
		AController* PC = DeadPawn->GetController();
		if (PC)
		{
			// 2. 입력 비활성화 (기존 로직 유지)
			AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC);
			if (MyPC) MyPC->SetPlayerEnabledState(false);
				// 3. 타이머 설정: RespawnDelay(3초) 후에 RequestRespawn 호출
			FTimerHandle RespawnTimerHandle;
			FTimerDelegate RespawnDelegate;
			RespawnDelegate.BindUObject(this, &AArrowGameGameMode::RequestRespawn, PC);
			GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);
		}
	}
	
	
}


void AArrowGameGameMode::RequestRespawn(AController* Controller)
{
	if (bRoundEnded) return;
	
	if (Controller)
	{
		// 4. 엔진 기본 기능: 플레이어를 새로운 StartPoint에서 부활시키고 Pawn을 빙의(Possess)시킴
		RestartPlayer(Controller);

		// 5. 부활 후 다시 입력 활성화
		AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(Controller);
		if (MyPC) MyPC->SetPlayerEnabledState(true);
	}
}


void AArrowGameGameMode::EndRound(bool bDokkaebiWin)
{
	if (bRoundEnded) return;
	bRoundEnded = true;
	
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	GetWorldTimerManager().ClearTimer(LastThirtySecondsTimerHandle);
	
	UE_LOG(LogTemp, Warning, TEXT("Round Ended! Winner: %s"),
		bDokkaebiWin ? TEXT("Dokkaebi") : TEXT("Humans"));
	// 전원 입력 잠금
	for (APlayerController* PC : PendingPlayers)
	{
		if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
		{
			MyPC->SetPlayerEnabledState(false);
		}
	}
	
	ShowRoundResultToAll(bDokkaebiWin);
	ScheduleReturnToLobby(8.0f);
}

void AArrowGameGameMode::ShowRoundResultToAll(bool bDokkaebiWin)
{
	if (!bRoundEnded) return;
	
	for (APlayerController* PC : PendingPlayers)
	{
		AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC);
		if (!MyPC)
		{
			UE_LOG(LogTemp, Error, TEXT("ShowRoundResultToAll: Non-ArrowGamePlayerController detected: %s"), *GetNameSafe(PC));
			continue;
		}
		AArrowPlayerState* ArrowPS = MyPC->GetPlayerState<AArrowPlayerState>();
		if (!ArrowPS) continue;
		
		const bool bIsWinner = (ArrowPS->IsDokkaebi() == bDokkaebiWin);
		
		MyPC->Client_ShowRoundResult(bIsWinner, 8.0f);
	}
}

void AArrowGameGameMode::ScheduleReturnToLobby(float Delay)
{
    FTimerHandle ReturnHandle;
    GetWorldTimerManager().SetTimer(ReturnHandle, [this]()
    {
        if (!HasAuthority()) return;
        GetWorld()->ServerTravel(TEXT("/Game/ArrowGame/Maps/LobbyMap?listen?game=/Script/ArrowGame.LobbyGameMode"));
    }, Delay, false);
}

void AArrowGameGameMode::OnRoundTimeUp()
{
	if (bRoundEnded) return;
	UE_LOG(LogTemp, Warning, TEXT("시간 초과! 살아남은 도깨비의 승리입니다."));
	EndRound(true);
}

void AArrowGameGameMode::OnLastThirtySecondsStart()
{
	if (bRoundEnded) return;
	if (AArrowGameState* GS = GetWorld()->GetGameState<AArrowGameState>())
	{
		GS->bLastThirtySeconds = true;
		UE_LOG(LogTemp, Warning, TEXT("마지막 30초! 아처에게 도깨비 위치 공개"));
	}
}
