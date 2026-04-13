#include "DokkaebiDecoy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ADokkaebiDecoy::ADokkaebiDecoy()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 서버에서 스폰되어 클라로 복제됨
    
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

	if (HasAuthority()) // 서버에서만 진실로 이동
	{
		AddMovementInput(GetActorForwardVector(), 1.0f);
	}
}