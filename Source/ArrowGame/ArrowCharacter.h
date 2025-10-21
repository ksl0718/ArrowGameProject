// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArrowCharacter.generated.h"

UCLASS()
class ARROWGAME_API AArrowCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AArrowCharacter();

    virtual void Die();

protected:

    virtual void BeginPlay() override;
    virtual void FireArrow(const FRotator& FireDir, float Speed = 3000.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<class AArrowProjectile> ArrowProjectileClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    USceneComponent* ProjectileSpawnPoint;

    void HandleDeath();

    virtual void OnDeath();

    bool bIsDead = false;
};
