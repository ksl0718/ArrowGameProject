#include "DokkaebiDecoy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ADokkaebiDecoy::ADokkaebiDecoy()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);


    // CharacterMovement가 네트워크 스무딩/충돌을 담당하게 하는 게 핵심
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 500.f, 0.f);
        Move->bRunPhysicsWithNoController = true;  // 컨트롤러 없어도 이동 물리 수행
        Move->SetMovementMode(MOVE_Walking);
        Move->GravityScale = 1.f;
    }

	// 발사체 통과 설정 (기획 선택 반영)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void ADokkaebiDecoy::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(3.0f); // 3초 뒤 자동 소멸

    GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ADokkaebiDecoy::OnCapsuleHit);
}

void ADokkaebiDecoy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	if (HasAuthority() && !bStoppedByHit)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->MaxWalkSpeed = DecoySpeed;

			// 수평만 지정하고 Z는 CMC 중력/낙하에 맡김 (Velocity 전체 덮어쓰면 절벽에서 안 떨어짐)
			FVector Vel = Move->Velocity;
			const FVector ForwardFlat = FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.f);
			if (!ForwardFlat.IsNearlyZero())
			{
				const FVector HorizontalVel = ForwardFlat.GetSafeNormal() * DecoySpeed;
				Vel.X = HorizontalVel.X;
				Vel.Y = HorizontalVel.Y;
			}
			Move->Velocity = Vel;
		}
	}
}

void ADokkaebiDecoy::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	if (bStoppedByHit)
	{
		return;
	}

	if (Hit.bBlockingHit && Hit.ImpactNormal.Z < 0.5f)
	{
		bStoppedByHit = true;
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->Velocity = FVector::ZeroVector;
		}
	}
}