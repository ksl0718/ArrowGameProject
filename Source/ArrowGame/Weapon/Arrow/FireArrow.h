#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "FireArrow.generated.h"

UCLASS()
class ARROWGAME_API AFireArrow : public AArrowProjectile
{
	GENERATED_BODY()

public:
	AFireArrow();

protected:
	virtual void NotifyImpact(const FHitResult& Hit) override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnFireFX(FVector Location);

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire | FX")
	class UNiagaraSystem* FireFX;

	UPROPERTY(EditAnywhere, Category = "Fire | FX")
	class USoundBase* FireImpactSound;
};
