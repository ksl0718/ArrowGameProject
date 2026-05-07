#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DokkaebiCurseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class ARROWGAME_API ADokkaebiCurseProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	ADokkaebiCurseProjectile();
	
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(EditAnywhere, Category="Curse")
	float LifeSeconds = 2.0f;
	
	UPROPERTY(EditAnywhere, Category="Curse")
	float CurseDuration = 2.5f;
};
