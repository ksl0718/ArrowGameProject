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
	float BurnDamage = 6.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = "Fire | Settings")
	float BurnDuration = 10.f;

	/** 캐릭터 직격 시 명중 지점 3D 사운드 — Sound Cue Attenuation (전 클라이언트) */
	UPROPERTY(EditAnywhere, Category = "Fire | FX")
	class USoundBase* FireImpactSound;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFireImpactSound(FVector Location);

	UPROPERTY(EditAnywhere, Category = "Fire | Sound")
	class USoundBase* LaunchSound;

	/** 바닥 화염 존 루프음 — FireZoneActor에 3D로 재생 (가까울 때만 들림) */
	UPROPERTY(EditAnywhere, Category = "Fire | Sound")
	class USoundBase* GroundFireLoopSound;

};
