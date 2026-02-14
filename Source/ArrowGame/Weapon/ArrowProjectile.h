// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"  
#include "Components/BoxComponent.h"                     
#include "Components/StaticMeshComponent.h"              
#include "NiagaraComponent.h"                            
#include "ArrowProjectile.generated.h"

UCLASS()
class ARROWGAME_API AArrowProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	
	AArrowProjectile();
	
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* TrailNiagara;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	void FireInDirection(const FVector& ShootDirection);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* ArrowMesh;

	UFUNCTION(BlueprintCallable)
	void InitVelocity(const FVector& Velocity);

private:
	UPROPERTY(EditAnyWhere)
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
	float Damage = 50.f;

	bool bStuck = false;
	
	FVector PrevLocation; 
	
	void StopAndDisable();  // ȭ�� ���߰� �浹 ���� ���� ó��
	void StickIntoCharacter(APawn* HitPawn, UPrimitiveComponent* OtherComp, const FHitResult& Hit);
	void HitPhysicsObject(UPrimitiveComponent* OtherComp, const FHitResult& Hit, AActor* MyOwner);
	void StickIntoWorld(UPrimitiveComponent* OtherComp, AActor* OtherActor, const FHitResult& Hit);

};
