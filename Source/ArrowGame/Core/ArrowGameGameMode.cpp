// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGameGameMode.h"
#include "../Character/UserCharacter.h"
#include "ArrowGamePlayerController.h"
#include "../AI/AICharacter.h"
#include "Kismet/GameplayStatics.h"

void AArrowGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	//User = Cast<AUserCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	//ArrowGamePlayerController = Cast<AArrowGamePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}

void AArrowGameGameMode::ActorDied(AActor* DeadActor)
{
	// 1. 죽은 액터가 플레이어인지 확인
	APawn* DeadPawn = Cast<APawn>(DeadActor);
	if (DeadPawn && DeadPawn->IsPlayerControlled())
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
	if (Controller)
	{
		// 4. 엔진 기본 기능: 플레이어를 새로운 StartPoint에서 부활시키고 Pawn을 빙의(Possess)시킴
		RestartPlayer(Controller);

		// 5. 부활 후 다시 입력 활성화
		AArrowGamePlayerController* MyPC = Cast<AArrowGamePlayerController>(Controller);
		if (MyPC) MyPC->SetPlayerEnabledState(true);
	}
}