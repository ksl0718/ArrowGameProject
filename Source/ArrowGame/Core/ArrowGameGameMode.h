// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArrowGameGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API AArrowGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void ActorDied(AActor* DeadActor);
	virtual void BeginPlay() override;

private:

	class AUserCharacter* User;
	class AArrowGamePlayerController* ArrowGamePlayerController;


};
