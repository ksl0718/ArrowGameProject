#include "DokkaebiCharacterAnimInstance.h"
#include "DokkaebiCharacter.h"
#include "DokkaebiDecoy.h"
#include "CharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UDokkaebiCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	bIsDead = false;
	bIsStealthed = false;
	bCurseOrbReady = false;

	if (ACharacterBase* CharBase = Cast<ACharacterBase>(Pawn))
	{
		bIsDead = CharBase->bIsDead;
	}

	if (Cast<ADokkaebiDecoy>(Pawn))
	{
		const float RawSpeed = Character->GetVelocity().Size2D();
		Speed = FMath::FInterpTo(Speed, RawSpeed, DeltaSeconds, 10.f);
		GroundSpeed = Speed;
		Direction = 0.f;
		bShouldMove = Speed > 3.f;
		Pitch = 0.f;
		bIsInAir = false;
		bIsCrouched = false;
		return;
	}

	if (ADokkaebiCharacter* Dokkaebi = Cast<ADokkaebiCharacter>(Pawn))
	{
		bIsStealthed = Dokkaebi->IsStealthed();
		bCurseOrbReady = Dokkaebi->IsCurseOrbReady();

		float TargetPitch = 0.f;
		if (Dokkaebi->IsLocallyControlled())
		{
			TargetPitch = FRotator::NormalizeAxis(Dokkaebi->GetControlRotation().Pitch);
		}
		else
		{
			TargetPitch = Dokkaebi->GetSyncPitch();
		}
		TargetPitch = FMath::Clamp(TargetPitch, -90.f, 90.f);

		const float InterpSpeed = Dokkaebi->IsLocallyControlled() ? 0.f : 15.f;
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
	}
	else
	{
		Pitch = 0.f;
	}

	const FVector Velocity = Character->GetVelocity();
	float ComputedSpeed = Velocity.Size2D();
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

	Speed = ComputedSpeed;
	GroundSpeed = Speed;
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	bShouldMove = Speed > 3.f;

	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		bIsInAir = MoveComp->IsFalling();
	}
	else
	{
		bIsInAir = false;
	}
	bIsCrouched = Character->bIsCrouched;
}

void UDokkaebiCharacterAnimInstance::SetCanMove(bool bNewCanMove)
{
	if (ADokkaebiCharacter* Dokkaebi = Cast<ADokkaebiCharacter>(TryGetPawnOwner()))
	{
		Dokkaebi->SetCanMove(bNewCanMove);
	}
}
