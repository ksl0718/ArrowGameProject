// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArrowCharacter.generated.h"

class AWeapon;

UCLASS()
class ARROWGAME_API AArrowCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AArrowCharacter();

    virtual void Die();

    UFUNCTION(BlueprintCallable)
    virtual void EquipWeapon(AWeapon* NewWeapon);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<AWeapon> DefaultWeaponClass;
    void PlayFireMontage();

protected:

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "stats")
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AWeapon* EquippedWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
    class UAnimMontage* FireMontage;

	/*void PlayFireMontage();*/
    void HandleDeath();

    virtual void OnDeath();
};
