#include "DokkaebiDecoy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ADokkaebiDecoy::ADokkaebiDecoy()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// 발사체 통과 설정 (기획 선택 반영)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void ADokkaebiDecoy::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(3.0f); // 3초 뒤 자동 소멸
}

void ADokkaebiDecoy::Tick(float DeltaTime)
{
	if (HasAuthority())
    {
        FVector Forward = GetActorForwardVector();
        Forward.Z = 0.f;
        Forward = Forward.GetSafeNormal();
        const FVector Delta = Forward * DecoySpeed * DeltaTime;
        FHitResult Hit;
        AddActorWorldOffset(Delta, true, &Hit);
        if (Hit.bBlockingHit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Decoy blocked by: %s"),
                *GetNameSafe(Hit.GetActor()));
        }
    }
}