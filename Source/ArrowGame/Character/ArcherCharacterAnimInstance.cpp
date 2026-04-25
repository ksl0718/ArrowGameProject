#include "ArcherCharacterAnimInstance.h"
#include "ArcherCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UArcherCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	bHasEquippedBow = false;
	if (AArcherCharacterBase* Archer = Cast<AArcherCharacterBase>(Pawn))
	{
		bHasEquippedBow = Archer->HasEquippedBow();
	}

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!Character) return;
	
	const FVector Velocity = Character->GetVelocity();
	Speed = Velocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	
	bShouldMove = Speed > 3.f;
	
	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsCrouched = Character->bIsCrouched;
}