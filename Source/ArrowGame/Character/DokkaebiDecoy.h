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

	UFUNCTION()
    void OnCapsuleHit(
        UPrimitiveComponent* HitComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

public:
	virtual void Tick(float DeltaTime) override;

	// 분신 이동 속도
	UPROPERTY(EditAnywhere, Category = "Decoy")
	float DecoySpeed = 450.f;

	
private:
    // 한번 막히면 이후엔 속도 갱신을 안 해서 "벽에 비비며 떨림" 방지
    bool bStoppedByHit = false;
};
