// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGameGameMode.h"
#include "ArrowGame/Core/ArrowGamePlayerController.h"
#include "ArrowGame/Core/ArrowPlayerState.h"


void AArrowGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	//User = Cast<AUserCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	//ArrowGamePlayerController = Cast<AArrowGamePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}

void AArrowGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	PendingPlayers.Add(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("플레이어 접속: %s | 현재 인원: %d / %d"), 
	   *NewPlayer->GetName(), PendingPlayers.Num(), ExpectedPlayers);
    
	if (PendingPlayers.Num() >= ExpectedPlayers && !bGameStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 인원 집합! 게임 시작 시작!"));
		AssignDokkaebiAndStart();
	}
}

void AArrowGameGameMode::AssignDokkaebiAndStart()
{
	bGameStarted = true;
	
	int32 LuckyIdx = FMath::RandRange(0, PendingPlayers.Num() - 1);
	
	for (int32 i = 0; i < PendingPlayers.Num(); i++)
	{
		APlayerController* PC = PendingPlayers[i];
		if (AArrowPlayerState* PS = PC->GetPlayerState<AArrowPlayerState>())
		{
			PS -> SetIsDokkaebi(i == LuckyIdx);
			// 여기서 확인 로그를 찍어보세요!
			UE_LOG(LogTemp, Warning, TEXT("플레이어 %d번(%s) 도깨비 설정: %s"), 
				i, *PC->GetName(), PS->IsDokkaebi() ? TEXT("True") : TEXT("False"));
		}
		
		if (PC->GetPawn()) 
		{
			PC->GetPawn()->Destroy();
		}
		
		RestartPlayer(PC); // 캐릭터 생성
		if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
		{
			MyPC -> SetPlayerEnabledState(false);
		}
		
	}
	FTimerHandle UIStartHandle;
	GetWorldTimerManager().SetTimer(UIStartHandle, [this]()
	{
		for (APlayerController* PC : PendingPlayers)
		{
			if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
			{
				MyPC->Client_StartCountdown(5.0f); // 이제서야 명령 전송!
			}
		}

		// 실제 게임 시작(봉인 해제) 타이머는 여기서부터 5초 뒤에 실행
		GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AArrowGameGameMode::ActualStartGame, 5.0f, false);
        
	}, 1.0f, false); // 1.0초의 대기 시간 부여
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

	for (APlayerController* PC : PendingPlayers)
	{
		if (AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(PC))
		{
			MyPC -> SetPlayerEnabledState(true);
			MyPC -> Client_BattleStart(RoundTimeLimit);
		}
	}
	
	// [추가] 실제 배틀이 시작되면 제한 시간 타이머를 돌립니다.
	UE_LOG(LogTemp, Warning, TEXT("라운드 타이머 시작! 제한 시간: %f초"), RoundTimeLimit);
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AArrowGameGameMode::OnRoundTimeUp, RoundTimeLimit, false);
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
	
	// 라운드 카운트 강제 제거
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	
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
        // 실제 로비 맵 경로로 바꿔야 함
        GetWorld()->ServerTravel(TEXT("/Game/ArrowGame/Maps/LobbyMap?listen"));
    }, Delay, false);
}

void AArrowGameGameMode::OnRoundTimeUp()
{
	// 혹시라도 이미 라운드가 끝났다면 무시
	if (bRoundEnded) return;

	UE_LOG(LogTemp, Warning, TEXT("시간 초과! 살아남은 도깨비의 승리입니다."));
    
	// 도깨비 승리(true)로 라운드 종료
	EndRound(true); 
}