#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "FireArrow.generated.h"

class AFireZoneActor;

UCLASS()
class ARROWGAME_API AFireArrow : public AArrowProjectile
{
	GENERATED_BODY()

public:
	AFireArrow();

protected:
	virtual void NotifyImpact(const FHitResult& Hit) override;
	virtual void PlayLaunchEffects() override;

	void SpawnFireZone(const FHitResult& Hit);
	void ApplyBurnToPawn(AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFireImpactSound(FVector Location);

	UPROPERTY(EditDefaultsOnly, Category = "Fire | Zone")
	TSubclassOf<AFireZoneActor> FireZoneClass;

	UPROPERTY(EditAnywhere, Category = "Fire | Zone")
	float GroundFireRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Zone")
	float GroundFireLifetime = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire | FX")
	class UNiagaraSystem* GroundFireFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire | FX")
	class UNiagaraSystem* BodyBurnFX;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Fire | FX")
	class USoundBase* FireImpactSound;

	UPROPERTY(EditAnywhere, Category = "Fire | Sound")
	class USoundBase* LaunchSound;

};
