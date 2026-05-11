// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"  
#include "Components/BoxComponent.h"                     
#include "Components/StaticMeshComponent.h"              
#include "NiagaraComponent.h"                            
#include "ArrowProjectile.generated.h"

UENUM(BlueprintType)
enum class EArrowType : uint8
{
	Normal      UMETA(DisplayName = "Normal Arrow"),
	Fire        UMETA(DisplayName = "Fire Arrow"),
	Explosive   UMETA(DisplayName = "Explosive Arrow"),
	Max         UMETA(Hidden) 
};

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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateTrail();
	
	UFUNCTION(BlueprintCallable, Category = "Arrow Data")
	EArrowType GetArrowType() const { return ArrowType; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Data")
	EArrowType ArrowType = EArrowType::Normal;
	
	UPROPERTY(EditAnyWhere)
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
	float Damage = 50.f;

	UPROPERTY(ReplicatedUsing = OnRep_bStuck)
	bool bStuck = false;

	UFUNCTION()
	void OnRep_bStuck();

	FVector PrevLocation;
	
	void StopAndDisable();  // ȭ�� ���߰� �浹 ���� ���� ó��
	void StickIntoCharacter(APawn* HitPawn, UPrimitiveComponent* OtherComp, const FHitResult& Hit);
	void HitPhysicsObject(UPrimitiveComponent* OtherComp, const FHitResult& Hit, AActor* MyOwner);
	void StickIntoWorld(UPrimitiveComponent* OtherComp, AActor* OtherActor, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bShouldApplyDirectDamage = true;
	
	virtual void NotifyImpact(const FHitResult& Hit);

	static bool IsEnemy(APawn* Instigator, AActor* Target);

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

	// 캐릭터가 주울 때 부를 함수
	void PickUp(class AArcherCharacterBase* Picker);
    
	// 박혀있는 상태인지 확인
	bool IsStuck() const { return bStuck; }
	
	
private:
	

};
