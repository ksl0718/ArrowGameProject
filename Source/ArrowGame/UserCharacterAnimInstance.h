// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UserCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API UUserCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool IsAiming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool IsCharging = false;
};
