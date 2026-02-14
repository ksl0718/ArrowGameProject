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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	AArrowCharacter();

    virtual void Die();

	virtual void Tick(float DeltaTime) override;
	
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
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	
	void SetAiming(bool bNewAiming);
	
	float GetSyncPitch() const { return SyncPitch; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UHealthComponent* HealthComp;
	
protected:

    virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sync")
	float SyncPitch;
	
    UPROPERTY(BlueprintReadOnly, Category = "stats")
    bool bIsRolling = false; // 구르기 상태

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "stats")
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AWeapon* EquippedWeapon;

	void SetRotationMode(bool bAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);
	
	UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming;
	
	UFUNCTION()
	void OnRep_IsAiming();
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	// [추가] 컴포넌트에서 "죽었다"고 신호 오면 실행할 함수
	UFUNCTION()
	void OnDeathProcessed();

	// [추가] 멀티캐스트: 모두에게 래그돌 실행 명령
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Die();
	
	/*void PlayFireMontage();*/
    void HandleDeath();

    virtual void OnDeath();
};
