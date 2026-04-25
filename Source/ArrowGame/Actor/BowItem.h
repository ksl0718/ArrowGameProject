#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BowItem.generated.h"

UCLASS()
class ARROWGAME_API ABowItem : public AActor
{
	GENERATED_BODY()
    
public: 
	ABowItem();

	// 상호작용 시 호출
	void PickUp(class AUserArcherCharacter* Picker);
	void PickUp(class AUserCharacter* Picker);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USkeletalMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* CollisionBox;

	// 이 아이템을 주우면 어떤 활 클래스를 생성할 것인가?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class ABow> BowClass;
	
	virtual void OnConstruction(const FTransform& Transform) override;
};