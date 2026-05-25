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
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	void ActorDied(AActor* DeadActor, AController* KillerController);
	
	
	void RequestRespawn(AController* Controller);

protected:
	virtual void BeginPlay() override;
	
	void AssignDokkaebiAndStart();
	void ActualStartGame();
	
	void EndRound(bool bDokkaebiWin);//true면 도깨비 승리, false면 인간 승리

	void ShowRoundResultToAll(bool bDokkaebiWin);

	void ScheduleReturnToLobby(float Delay);
	
	// 라운드 제한 시간 (에디터에서 쉽게 수정 가능하도록 UPROPERTY 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	float RoundTimeLimit = 180.0f;

	// 제한 시간 타이머를 잡고 있을 손잡이
	FTimerHandle RoundTimerHandle;

	// 시간이 다 지났을 때 호출될 함수
	void OnRoundTimeUp();
	
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
	
	bool bRoundEnded = false;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float RespawnDelay = 3.0f;
};
