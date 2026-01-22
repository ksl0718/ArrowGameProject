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
	//GetPlayerCharacter(0)은 서버 입장에서 서버 주인(Host) 한 명만 가리킴
	//친구(Client 1)가 들어와서 죽으면, 게임모드는 DeadActor == User(호스트)가 아니라고 판단하여 친구의 사망 처리를 무시
	//친구는 HP가 0이 되어도 안 죽는 버그맨...
}

void AArrowGameGameMode::ActorDied(AActor* DeadActor)
{
	// 죽은 액터가 유저 캐릭터인지 확인
	if (AUserCharacter* DeadUser = Cast<AUserCharacter>(DeadActor))
	{
		// 유저 사망 처리 (서버 -> 클라 전파)
		DeadUser->Die();

		// 해당 유저의 컨트롤러를 찾아서 입력 차단
		// (Controller는 Pawn이 가지고 있으므로 바로 접근 가능)
		if (AArrowGamePlayerController* PC = Cast<AArrowGamePlayerController>(DeadUser->GetController()))
		{
			PC->SetPlayerEnabledState(false);
		}
	}
	// 죽은 액터가 AI인지 확인
	else if (AAICharacter* DestroyedCharacter = Cast<AAICharacter>(DeadActor))
	{
		DestroyedCharacter->Die();
	}
}
