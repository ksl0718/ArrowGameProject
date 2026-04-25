#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Weapon/ArrowProjectile.h" //
#include "ArrowItem.generated.h"

UCLASS()
class ARROWGAME_API AArrowItem : public AActor
{
	GENERATED_BODY()
    
public: 
	AArrowItem();

	// 캐릭터가 F키를 눌러 상호작용할 때 호출될 함수
	void PickUp(class AArcherCharacterBase* Picker);
	void PickUp(class AArrowCharacter* Picker);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;
	
protected:
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	// 이 아이템의 화살 종류와 지급 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EArrowType ArrowType = EArrowType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount = 5; 
};