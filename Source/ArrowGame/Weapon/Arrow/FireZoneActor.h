#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireZoneActor.generated.h"

class USphereComponent;
class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;
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
		UNiagaraSystem* InGroundFX, UNiagaraSystem* InBodyBurnFX,
		USoundBase* InLoopSound = nullptr);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* BurnSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* GroundFireNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* FireLoopAudio;

	/** 불 위치 3D 루프 사운드. 각 클라이언트가 카메라 거리에 따라 들음. */
	UPROPERTY()
	USoundBase* GroundFireLoopSound = nullptr;

	/** 미설정 시 사운드 에셋 기본 감쇠 사용. Max Distance 등은 여기서 조절. */
	UPROPERTY(EditAnywhere, Category = "Fire | Sound")
	USoundAttenuation* FireSoundAttenuation = nullptr;

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
	void StartFireLoopSound();
	void StopFireLoopSound();
};
