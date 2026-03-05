#include "ArrowItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../Character/ArrowCharacter.h"

AArrowItem::AArrowItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이 동기화

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	// 레이저(LineTrace)에 맞아야 하므로 Visibility 채널을 Block으로 설정!
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); 
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AArrowItem::PickUp(AArrowCharacter* Picker)
{
	if (HasAuthority() && Picker)
	{
		Picker->AddAmmo(ArrowType, Amount); // 주머니에 추가
		Destroy(); // 맵에서 삭제
	}
}