// Fill out your copyright notice in the Description page of Project Settings.


#include "UserCharacterAnimInstance.h"
#include "../Character/UserCharacter.h"
#include "../Character/ArrowCharacter.h"
#include "../Character/CharacterBase.h"
#include "../Weapon/Weapon.h"
#include "../Weapon/Bow.h"
#include "Kismet/KismetMathLibrary.h" // 방향 계산용 수학 함수
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UUserCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    APawn* Pawn = TryGetPawnOwner(); 
    if (!Pawn) return;

    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (!Character) return;

    if (ACharacterBase* CharBase = Cast<ACharacterBase>(Pawn))
    {
        bIsDead = CharBase->bIsDead;
    }
    else
    {
        bIsDead = false;
    }

    const FVector Velocity = Character->GetVelocity();
    float ComputedSpeed = Velocity.Size2D();
    // Velocity가 0으로 남는 경우(수동 이동 분신) 대비
    if (ComputedSpeed <= KINDA_SMALL_NUMBER)
    {
        const FVector CurrentLoc = Character->GetActorLocation();
        if (!bPrevLocationInitialized)
        {
            PrevWorldLocation = CurrentLoc;
            bPrevLocationInitialized = true;
        }
        if (DeltaSeconds > KINDA_SMALL_NUMBER)
        {
            const FVector Delta = CurrentLoc - PrevWorldLocation;
            ComputedSpeed = Delta.Size2D() / DeltaSeconds;
        }
        PrevWorldLocation = CurrentLoc;
    }
    GroundSpeed = ComputedSpeed;

    const FRotator ActorRotation = Character->GetActorRotation();
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRotation);

    if (AUserCharacter* Archer = Cast<AUserCharacter>(Pawn))
    {
        bIsAiming = Archer->IsAiming();

        float TargetPitch = 0.f;
        if (Archer->IsLocallyControlled())
        {
            TargetPitch = Archer->GetControlRotation().Pitch;
            TargetPitch = FRotator::NormalizeAxis(TargetPitch);
        }
        else
        {
            TargetPitch = Archer->GetSyncPitch();
        }

        TargetPitch = FMath::Clamp(TargetPitch, -90.0f, 90.0f);

        const float InterpSpeed = Archer->IsLocallyControlled() ? 0.0f : 15.0f;
        const FRotator CurrentRot = FRotator(Pitch, 0, 0);
        const FRotator GoalRot = FRotator(TargetPitch, 0, 0);
        if (InterpSpeed <= 0.f)
        {
            Pitch = TargetPitch;
        }
        else
        {
            Pitch = FMath::RInterpTo(CurrentRot, GoalRot, DeltaSeconds, InterpSpeed).Pitch;
        }

        if (Archer->GetEquippedWeapon())
        {
            if (ABow* Bow = Cast<ABow>(Archer->GetEquippedWeapon()))
            {
                bIsCharging = Bow->IsCharging();
                bIsReloading = Bow->IsReloading();
            }
        }
        else
        {
            bIsAiming = false;
            bIsCharging = false;
        }
    }
    else
    {
        bIsAiming = false;
        bIsCharging = false;
        bIsReloading = false;

        if (Character->IsLocallyControlled())
        {
            float TargetPitch = Character->GetControlRotation().Pitch;
            TargetPitch = FRotator::NormalizeAxis(TargetPitch);
            Pitch = FMath::Clamp(TargetPitch, -90.0f, 90.0f);
        }
        else
        {
            const FRotator CurrentRot = FRotator(Pitch, 0, 0);
            Pitch = FMath::RInterpTo(CurrentRot, FRotator::ZeroRotator, DeltaSeconds, 15.f).Pitch;
        }
    }

    bShouldMove = (GroundSpeed > 3.0f);
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
