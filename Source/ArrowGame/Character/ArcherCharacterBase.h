// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "CharacterBase.h"
#include "ArcherCharacterBase.generated.h"

class AWeapon;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class ARROWGAME_API AArcherCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AArcherCharacterBase();

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	class UAnimMontage* CancelMontage;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	void SetAiming(bool bNewAiming);

	float GetSyncPitch() const { return SyncPitch; }

	UFUNCTION(BlueprintCallable)
	bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable)
	bool IsRolling() const { return bIsRolling; }

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetAmmoCount(EArrowType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void ConsumeAmmo(EArrowType Type, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void AddAmmo(EArrowType Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	EArrowType GetCurrentArrowType() const { return CurrentArrowType; }

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	TSubclassOf<class AArrowProjectile> GetCurrentArrowClass() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Bow")
	bool HasEquippedBow() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sync")
	float SyncPitch;

	UPROPERTY(BlueprintReadOnly, Category = "stats")
	bool bIsRolling = false;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	void ApplyAimingMovementSettings(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming;

	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerPlayCancelMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCancelMontage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentArrowType, BlueprintReadOnly, Category = "Inventory")
	EArrowType CurrentArrowType = EArrowType::Normal;

	UFUNCTION()
	void OnRep_CurrentArrowType();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory")
	TArray<int32> ArrowAmmoCounts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TMap<EArrowType, TSubclassOf<class AArrowProjectile>> ArrowClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TMap<EArrowType, int32> StartingAmmo;

	UFUNCTION(Server, Reliable)
	void ServerChangeArrowType(EArrowType NewType);

	void EquipArrow(EArrowType NewType);

	/** BP Components 이름과 동일해야 함 (활 부모). */
	UPROPERTY(EditDefaultsOnly, Category = "Archer|Visual")
	FName BowInHandComponentName = FName(TEXT("BowInHand"));

	UPROPERTY(EditDefaultsOnly, Category = "Archer|Visual")
	FName SM_ArrowsComponentName = FName(TEXT("SM_Arrows"));

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Archer|Visual")
	TObjectPtr<USceneComponent> BowInHandAnchor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Archer|Visual")
	TObjectPtr<UStaticMeshComponent> SM_ArrowsMesh;

private:
	void CacheArcherVisualComponents();

	UActorComponent* FindOwnedComponentByInstanceName(FName InstanceName) const;
};
