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
	Super::Tick(DeltaTime);

	// 컨트롤러 없는 분신은 AddMovementInput이 먹지 않음 → 서버에서 직접 이동
	if (HasAuthority())
	{
		const FVector Delta = GetActorForwardVector() * DecoySpeed * DeltaTime;
		AddActorWorldOffset(Delta, true);
	}
}