#include "DokkaebiDecoy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"


ADokkaebiDecoy::ADokkaebiDecoy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	bReplicates = true;
	SetReplicateMovement(true);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Move->PrimaryComponentTick.AddPrerequisite(this, PrimaryActorTick);

		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 500.f, 0.f);
		Move->bRunPhysicsWithNoController = true;
		Move->SetMovementMode(MOVE_Walking);
		Move->GravityScale = 1.f;
		Move->MaxStepHeight = 45.f;
		Move->SetWalkableFloorAngle(44.f);
		Move->bUseFlatBaseForFloorChecks = true;
	}

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void ADokkaebiDecoy::SyncMovementFromOwner()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (const UCapsuleComponent* OwnerCapsule = OwnerCharacter->GetCapsuleComponent())
		{
			if (UCapsuleComponent* Capsule = GetCapsuleComponent())
			{
				Capsule->SetCapsuleSize(
					OwnerCapsule->GetUnscaledCapsuleRadius(),
					OwnerCapsule->GetUnscaledCapsuleHalfHeight());
			}
		}

		if (const UCharacterMovementComponent* OwnerMove = OwnerCharacter->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = OwnerMove->MaxWalkSpeed * SpeedMultiplier;
			Move->MaxAcceleration = OwnerMove->MaxAcceleration;
			Move->MaxStepHeight = OwnerMove->MaxStepHeight + ExtraStepHeight;
			Move->SetWalkableFloorAngle(OwnerMove->GetWalkableFloorAngle());
			Move->PerchRadiusThreshold = OwnerMove->PerchRadiusThreshold;
			Move->PerchAdditionalHeight = OwnerMove->PerchAdditionalHeight;
			Move->GroundFriction = OwnerMove->GroundFriction;
			Move->BrakingDecelerationWalking = OwnerMove->BrakingDecelerationWalking;
			return;
		}
	}

	Move->MaxWalkSpeed *= SpeedMultiplier;
	Move->MaxStepHeight += ExtraStepHeight;
}

void ADokkaebiDecoy::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(3.0f);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
	}

	SyncMovementFromOwner();
}

void ADokkaebiDecoy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	const FVector ForwardFlat(GetActorForwardVector().X, GetActorForwardVector().Y, 0.f);
	if (!ForwardFlat.IsNearlyZero())
	{
		AddMovementInput(ForwardFlat.GetSafeNormal(), 1.f);
	}
}
