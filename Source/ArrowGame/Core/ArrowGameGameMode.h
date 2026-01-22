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
	
	// [삭제] 특정 유저만 저장하는 변수는 멀티플레이에서 위험쓰
	// private:
	// class AUserCharacter* User;
	// class AArrowGamePlayerController* ArrowGamePlayerController;
	
};
