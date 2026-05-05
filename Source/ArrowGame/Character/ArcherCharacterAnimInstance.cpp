#include "ArcherCharacterAnimInstance.h"
#include "ArcherCharacterBase.h"
#include "ArrowGame/Weapon/Bow.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "UserArcherCharacter.h"

void UArcherCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	bHasEquippedBow = false;
	if (AUserArcherCharacter* Archer = Cast<AUserArcherCharacter>(Pawn))
	{
		bHasEquippedBow = Archer->HasEquippedBow();
	}

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!Character) return;

	if (AUserArcherCharacter* Archer = Cast<AUserArcherCharacter>(Pawn))
	{
		bIsDead = Archer->bIsDead;
		bIsAiming = Archer->IsAiming();
		isAiming = bIsAiming;

		float TargetPitch = 0.f;
		if (Archer->IsLocallyControlled())
		{
			TargetPitch = FRotator::NormalizeAxis(Archer->GetControlRotation().Pitch);
		}
		else
		{
			TargetPitch = Archer->GetSyncPitch();
		}
		TargetPitch = FMath::Clamp(TargetPitch, -90.0f, 90.0f);

		const float InterpSpeed = Archer->IsLocallyControlled() ? 0.0f : 15.0f;
		if (InterpSpeed <= 0.f)
		{
			Pitch = TargetPitch;
		}
		else
		{
			const FRotator CurrentRot(Pitch, 0.f, 0.f);
			const FRotator GoalRot(TargetPitch, 0.f, 0.f);
			Pitch = FMath::RInterpTo(CurrentRot, GoalRot, DeltaSeconds, InterpSpeed).Pitch;
		}

		bIsCharging = false;
		bIsReloading = false;
		if (ABow* Bow = Cast<ABow>(Archer->GetEquippedWeapon()))
		{
			bIsCharging = Bow->IsCharging();
			bIsReloading = Bow->IsReloading();
		}
		isCharging = bIsCharging;
		isReloading = bIsReloading;
	}
	else
	{
		bIsDead = false;
		bIsAiming = false;
		bIsCharging = false;
		bIsReloading = false;
		isAiming = false;
		isCharging = false;
		isReloading = false;
		Pitch = 0.f;
	}
	
	const FVector Velocity = Character->GetVelocity();
	Speed = Velocity.Size2D();
	GroundSpeed = Speed;
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	
	bShouldMove = Speed > 3.f;
	
	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsCrouched = Character->bIsCrouched;
}
