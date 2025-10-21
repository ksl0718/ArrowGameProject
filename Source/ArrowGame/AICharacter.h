// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArrowCharacter.h"
#include "AICharacter.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API AAICharacter : public AArrowCharacter
{
	GENERATED_BODY()
public:
	AAICharacter();

    void HandleDeath();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

    //공격 관련
    void TryAttackTarget();

    //타겟 탐지 관련
    void SearchForTarget();

    //공격 타이머 (공격 쿨타임)
    FTimerHandle AttackTimerHandle;

    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackInterval = 3.f;

    //타겟
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget = nullptr;

    //사거리
    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackRange = 2000.f;

    //회전 속도
    UPROPERTY(EditAnywhere, Category = "AI")
    float TurnInterpSpeed = 5.f;

    virtual void OnDeath();
};
