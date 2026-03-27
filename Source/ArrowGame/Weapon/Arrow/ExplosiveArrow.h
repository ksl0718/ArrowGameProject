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
	
	// 폭발 화살만의 전용 변수들
	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionDamage = 50.f;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	class UNiagaraSystem* ExplosionFX;
};