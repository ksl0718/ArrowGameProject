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
        // 필요시 조정: 분신이 점프/낙하 등 원치 않으면 관련 옵션도 건드릴 수 있음
        // Move->GravityScale = 1.0f;
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

	if (HasAuthority() && !bStoppedByHit )
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            // MaxWalkSpeed는 애니/이동 느낌 맞출 때 유용
            Move->MaxWalkSpeed = DecoySpeed;
            // 핵심: 위치를 직접 밀지 말고, CMC가 처리할 Velocity를 제공
            // 이렇게 해야 Character 네트워킹/스무딩 규칙과 충돌 안 함
            const FVector Forward = GetActorForwardVector();
            Move->Velocity = Forward * DecoySpeed;
        }
    }
}

void ADokkaebiDecoy::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor
    , UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
    {
        if (!HasAuthority()) return;

        if (!OtherActor || OtherActor == this) return;
        // 내 본체(소유자/인스티게이터)와의 충돌은 무시
        if (OtherActor == GetOwner() || OtherActor == GetInstigator())
        {
            return;
        }

        if (bStoppedByHit) return;

        // 월드(벽/지형 등)에 막히면 정지
        if (Hit.bBlockingHit)
        {
            if (Hit.ImpactNormal.Z < 0.5f)
            {
                bStoppedByHit = true;
                if (UCharacterMovementComponent* Move = GetCharacterMovement())
                {
                    Move->StopMovementImmediately();
                    Move->Velocity = FVector::ZeroVector;
                }
            }

        }
    }   