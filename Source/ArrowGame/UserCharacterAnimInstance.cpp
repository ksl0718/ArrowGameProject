// Fill out your copyright notice in the Description page of Project Settings.


#include "UserCharacterAnimInstance.h"
#include "UserCharacter.h"

void UUserCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    AUserCharacter* Character = Cast<AUserCharacter>(TryGetPawnOwner());
    if (Character)
    {
        IsAiming = Character->IsAiming();
        IsCharging = Character->IsCharging();
    }
}