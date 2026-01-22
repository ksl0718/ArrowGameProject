// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGameGameMode.h"
#include "../Character/UserCharacter.h"
#include "ArrowGamePlayerController.h"
#include "../AI/AICharacter.h"
#include "Kismet/GameplayStatics.h"

void AArrowGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	User = Cast<AUserCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	ArrowGamePlayerController = Cast<AArrowGamePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}

void AArrowGameGameMode::ActorDied(AActor* DeadActor)
{
	if (DeadActor == User)
	{
		User->Die();
		if (ArrowGamePlayerController)
		{
			ArrowGamePlayerController->SetPlayerEnabledState(false);
		}
	}
	else if (AAICharacter* DestroyedCharacter = Cast<AAICharacter>(DeadActor))
	{
		DestroyedCharacter->Die();
	}
}