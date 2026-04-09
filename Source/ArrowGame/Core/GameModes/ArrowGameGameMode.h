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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	void ActorDied(AActor* DeadActor, AController* KillerController);
	
	
	void RequestRespawn(AController* Controller);

protected:
	virtual void BeginPlay() override;
	
	void AssignDokkaebiAndStart();
	void ActualStartGame();
	
private:
	
	// 캐릭터 클래스 설정
	UPROPERTY(EditAnywhere, Category = "Battle Settings")
	TSubclassOf<APawn> HumanClass;

	UPROPERTY(EditAnywhere, Category = "Battle Settings")
	TSubclassOf<APawn> DokkaebiClass;

	UPROPERTY()
	TArray<class APlayerController*> PendingPlayers;

	FTimerHandle CountdownTimerHandle;
	int32 ExpectedPlayers = 2; // GameInstance 연동 권장
	bool bGameStarted = false;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float RespawnDelay = 3.0f;
};
