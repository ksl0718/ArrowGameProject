#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireZoneActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** 땅/벽 화염 존 — 오버랩 시 화상 DoT + 몸불 FX */
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
	TSet<TWeakObjectPtr<AActor>> OverlappingBurnTargets;

	UFUNCTION()
	void OnBurnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBurnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyBurnToActor(AActor* Target);
	void TickBurnZone();
};
