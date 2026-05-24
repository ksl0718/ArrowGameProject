#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireZoneActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** 땅/벽에 박힌 화살이 남기는 지면 화염 (1단계: FX + 수명만) */
UCLASS()
class ARROWGAME_API AFireZoneActor : public AActor
{
	GENERATED_BODY()

public:
	AFireZoneActor();

	void InitZone(float InRadius, float InLifetime, UNiagaraSystem* InGroundFX);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* BurnSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* GroundFireNiagara;

	float ZoneLifetime = 15.f;
};
