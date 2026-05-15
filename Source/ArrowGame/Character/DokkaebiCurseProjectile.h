#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DokkaebiCharacter.h"
#include "DokkaebiCurseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class ARROWGAME_API ADokkaebiCurseProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	ADokkaebiCurseProjectile();
	
	void SetCurseCaster(ADokkaebiCharacter* InCaster);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* TrailNiagara;
	
protected:
	virtual void BeginPlay() override;

	void TryApplyCurseToVictim(AActor* Candidate);

	UPROPERTY(Transient)
	TWeakObjectPtr<ADokkaebiCharacter> CurseCaster;

	bool bCurseApplied = false;

	UFUNCTION()
	void OnPawnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(EditAnywhere, Category="Curse")
	float LifeSeconds = 2.0f;
	
	UPROPERTY(EditAnywhere, Category="Curse")
	float CurseDuration = 2.5f;
};
