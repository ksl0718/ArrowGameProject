#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseFieldActor.generated.h"

UCLASS()
class ARROWGAME_API ABaseFieldActor : public AActor
{
	GENERATED_BODY()
    
public:    
	ABaseFieldActor();

protected:
	// 종류에 따라 메쉬를 바꿀 수 있도록 루트 컴포넌트 선언
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* Mesh;

	// 나중에 상자, 벽, 기둥 등을 나눌 수 있는 열거형(Enum)이 있으면 편합니다.
	// UPROPERTY(EditAnywhere, Category = "Settings")
	// EFieldObjectType ObjectType;
};