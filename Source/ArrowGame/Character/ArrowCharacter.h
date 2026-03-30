// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "CharacterBase.h"
#include "ArrowCharacter.generated.h"

class AWeapon;

UCLASS()
class ARROWGAME_API AArrowCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	AArrowCharacter();
	
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
	
	// 화살 개수를 확인하고 소비하는 함수들
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetAmmoCount(EArrowType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void ConsumeAmmo(EArrowType Type, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void AddAmmo(EArrowType Type, int32 Amount);
	
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	EArrowType GetCurrentArrowType() const { return CurrentArrowType; }
	
	// 현재 선택된 화살의 '블루프린트 클래스'를 반환 (스폰용)
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	TSubclassOf<class AArrowProjectile> GetCurrentArrowClass() const;
	
protected:

    virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sync")
	float SyncPitch;
	
    UPROPERTY(BlueprintReadOnly, Category = "stats")
    bool bIsRolling = false; // ������ ����

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

	// [�߰�] ��� Ŭ���̾�Ʈ���� ��Ÿ�� ��� ���
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCancelMontage();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed = 400.f;     // ?? ???

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 200.f;       // ????? ???? ?? ???? ???
	

	//----------------화살 관련----------//
	// 1. 현재 들고 있는 화살 타입 (동기화 필수)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentArrowType, BlueprintReadOnly, Category = "Inventory")
	EArrowType CurrentArrowType = EArrowType::Normal;

	UFUNCTION()
	void OnRep_CurrentArrowType();

	// 2. 화살 소지 개수 배열 (인덱스 = EArrowType의 정수값)
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory")
	TArray<int32> ArrowAmmoCounts;

	// 3. 에디터에서 [Normal -> BP_NormalArrow], [Fire -> BP_FireArrow] 짝지어주는 맵
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TMap<EArrowType, TSubclassOf<class AArrowProjectile>> ArrowClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TMap<EArrowType, int32> StartingAmmo;
	
	// 4. 화살 교체를 서버에 요청하는 함수
	UFUNCTION(Server, Reliable)
	void ServerChangeArrowType(EArrowType NewType);

	// 입력 키(1, 2, 3 또는 휠)에 바인딩할 함수들
	void EquipArrow(EArrowType NewType);
	
};
