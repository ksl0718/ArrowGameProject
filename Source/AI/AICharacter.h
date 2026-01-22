// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ArrowCharacter.h"
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

    //���� ����
    void TryAttackTarget();

    //Ÿ�� Ž�� ����
    void SearchForTarget();

    //���� Ÿ�̸� (���� ��Ÿ��)
    FTimerHandle AttackTimerHandle;
    FTimerHandle AttackCooldownHandle;
    FTimerHandle AIDrawTimerHandle;
    FTimerHandle AIFireTimerHandle;

	//���� ����
    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackInterval = 3.f;

    //Ÿ��
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget = nullptr;

    //��Ÿ�
    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackRange = 2000.f;

    //ȸ�� �ӵ�
    UPROPERTY(EditAnywhere, Category = "AI")
    float TurnInterpSpeed = 5.f;

    UPROPERTY()
    bool bCanAttack = true;

    virtual void OnDeath();

    void ResetAttackCooldown();
    void BeginAIAim();
    void BeginAIDraw();
    void EndAIDrawAndFire();
};
