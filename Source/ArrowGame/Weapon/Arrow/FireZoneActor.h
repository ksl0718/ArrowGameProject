#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireZoneActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class APawn;

/** 땅/벽 화염 존 — 반경 내 Pawn에게 주기 화상 */
UCLASS()
class ARROWGAME_API AFireZoneActor : public AActor
{
	GENERATED_BODY()

public:
	AFireZoneActor();

	void InitZone(APawn* InInstigator, float InRadius, float InLifetime,
		float InBurnDamage, float InBurnInterval, float InBurnDuration,
		UNiagaraSystem* InGroundFX, UNiagaraSystem* InBodyBurnFX);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* BurnSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* GroundFireNiagara;

	TWeakObjectPtr<APawn> DamageInstigator;
	UPROPERTY()
	UNiagaraSystem* BodyBurnFX;

	float ZoneLifetime = 15.f;
	float BurnDamage = 10.f;
	float BurnInterval = 0.5f;
	float BurnDuration = 5.f;

	FTimerHandle BurnTickHandle;
	TSet<TWeakObjectPtr<APawn>> PawnsInZoneLastTick;

	void GatherPawnsInBurnRadius(TArray<APawn*>& OutPawns) const;
	void ApplyBurnToActor(APawn* Target);
	void StopBurnOnActor(APawn* Target);
	void TickBurnZone();
};
