#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DokkaebiDecoy.generated.h"

UCLASS()
class ARROWGAME_API ADokkaebiDecoy : public ACharacter
{
	GENERATED_BODY()
public:
	ADokkaebiDecoy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 분신 이동 속도
	UPROPERTY(EditAnywhere, Category = "Decoy")
	float DecoySpeed = 600.f;
	
};
