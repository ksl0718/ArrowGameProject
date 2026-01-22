// Fill out your copyright notice in the Description page of Project Settings.


#include "UserCharacterAnimInstance.h"
#include "UserCharacter.h"

void UUserCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

}

void UUserCharacterAnimInstance::SetCanMove(bool bNewCanMove)
{
    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        AUserCharacter* Character = Cast<AUserCharacter>(Pawn);
        if (Character)
        {
            Character->bCanMove = bNewCanMove;
        }
    }
}
