#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "ExplosiveArrow.generated.h"

UCLASS()
class ARROWGAME_API AExplosiveArrow : public AArrowProjectile
{
	GENERATED_BODY()

public:
	AExplosiveArrow();

protected:
	virtual void NotifyImpact(const FHitResult& Hit) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnExplosionFX(FVector Location);

	UPROPERTY(EditAnywhere, Category = "Explosion | Settings")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Explosion | Settings")
	float ExplosionDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion | FX")
	class UNiagaraSystem* ExplosionFX;

	UPROPERTY(EditAnywhere, Category = "Explosion | FX")
	class USoundBase* ExplosionSound;
};
