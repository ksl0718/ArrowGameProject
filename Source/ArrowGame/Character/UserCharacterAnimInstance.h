// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAnimInstanceBase.h"
#include "UserCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API UUserCharacterAnimInstance : public UCharacterAnimInstanceBase
{
	GENERATED_BODY()
public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable)
    void SetCanMove(bool bNewCanMove);
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsDead = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsAiming = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsCharging = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f; // 이동 방향 (블렌드 스페이스 X축)

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f; // 이동 속도 (블렌드 스페이스 Y축)

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float Pitch = 0.f; // 위아래 보는 각도 (에임 오프셋)
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false;
	

	UPROPERTY(Transient)
	FVector PrevWorldLocation = FVector::ZeroVector;
	
	UPROPERTY(Transient)
	bool bPrevLocationInitialized = false;
};
