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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
    virtual void Die();

    UFUNCTION(BlueprintCallable)
    virtual void EquipWeapon(AWeapon* NewWeapon);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<AWeapon> DefaultWeaponClass;

    UFUNCTION(BlueprintCallable)
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
    class UAnimMontage* FireMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
    class UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
    class UAnimMontage* HitMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
    class UAnimMontage* RollMontage;
protected:

    virtual void BeginPlay() override;
	
	// 서버에서 값이 바뀌면 클라이언트에서 OnRep_IsDead 자동 실행
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_IsDead)
    bool bIsDead = false;

	// 사망 상태가 복제되었을 때 실행되는 함수 (시각적 처리)
	UFUNCTION()
	void OnRep_IsDead();

	UPROPERTY(BlueprintReadOnly, Category = "stats")
	bool bIsRolling = false; // 구르기 상태
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AWeapon* EquippedWeapon;

    

	/*void PlayFireMontage();*/
    void HandleDeath();

    virtual void OnDeath();
};
