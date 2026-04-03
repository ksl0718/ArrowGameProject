#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ArrowGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);

UCLASS()
class ARROWGAME_API AArrowGameState : public AGameStateBase // 부모를 AGameState로 변경
{
	GENERATED_BODY()

public:
public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPlayerListChanged OnPlayerListChanged;

	// 1. 명단 변경 신호를 보낼 리플리케이션 변수
	UPROPERTY(ReplicatedUsing = OnRep_ListUpdateCount)
	int32 ListUpdateCount = 0;

	UFUNCTION()
	void OnRep_ListUpdateCount();

	// 2. 엔진 소스에 확인된 virtual 함수 오버라이드
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// 리플리케이션 설정 필수 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};