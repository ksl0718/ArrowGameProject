// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAnimInstanceBase.h"
#include "ArcherCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API UArcherCharacterAnimInstance : public UCharacterAnimInstanceBase
{
	GENERATED_BODY()
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouched = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsDead = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsAiming = false;

	/** Legacy ABP compatibility alias for old variable name. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool isAiming = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsReloading = false;

	/** Legacy ABP compatibility alias for old variable name. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool isReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsCharging = false;

	/** Legacy ABP compatibility alias for old variable name. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool isCharging = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f; // 이동 방향 (블렌드 스페이스 X축)

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.f; // 이동 속도 (블렌드 스페이스 Y축)

	/** Legacy ABP compatibility alias for old variable name. */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")	
	float Pitch = 0.f; // 위아래 보는 각도 (에임 오프셋)
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false;

	/** EquippedWeapon 이 ABow 인지(BP Bow 포함). */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	bool bHasEquippedBow = false;

	UPROPERTY(Transient)
	FVector PrevWorldLocation = FVector::ZeroVector;
	
	UPROPERTY(Transient)
	bool bPrevLocationInitialized = false;
};
