#include "BowItem.h"
#include "../Weapon/Bow.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../Character/UserCharacter.h"

ABowItem::ABowItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
    
	// 화살이랑 똑같이 설정 (센서에 걸려야 하니까)
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); 
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABowItem::PickUp(AUserCharacter* Picker)
{
	if (Picker)
	{
		// 여기서 캐릭터의 장착 함수를 호출!
		Picker->EquipNewBow(BowClass);
        
		// 장착시켰으니 아이템은 삭제
		Destroy();
	}
}

void ABowItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 1. 설정된 활 클래스가 있는지 확인
	if (BowClass)
	{
		// 2. [마법의 구간] 활 클래스의 '기본 정보(CDO)'를 가져옵니다.
		// 실제로 스폰하지 않고도 그 클래스에 저장된 기본값을 읽을 수 있습니다.
		ABow* DefaultBow = BowClass->GetDefaultObject<ABow>();
		
		if (DefaultBow)
		{
			// 2. [치트키] 이름은 모르겠지만, 어쨌든 '스켈레탈 메쉬'를 쓰고 있다면 찾아와라!
			USkeletalMeshComponent* BowMesh = DefaultBow->FindComponentByClass<USkeletalMeshComponent>();
            
			if (BowMesh && ItemMesh)
			{
				// 바닥 아이템(StaticMesh)에 활의 외형을 입힘
				// 스켈레탈 메쉬에서 스태틱 메쉬 정보만 쏙 빼오는 법
				ItemMesh->SetSkeletalMeshAsset(BowMesh->GetSkeletalMeshAsset());
			}
		}
	}
}