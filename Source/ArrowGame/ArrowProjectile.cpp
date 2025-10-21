// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "ArrowCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AArrowProjectile::AArrowProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
    RootComponent = ArrowMesh;

    ArrowMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    ArrowMesh->SetNotifyRigidBodyCollision(true);
    ArrowMesh->SetSimulatePhysics(false);

	TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	TrailNiagara->SetupAttachment(RootComponent);
	TrailNiagara->bAutoActivate = false;

	/*TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	TrailNiagara->SetupAttachment(RootComponent);*/

    // 충돌 이벤트 연결
    ArrowMesh->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnHit);

    // 움직임 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.5f; // 화살 낙차용

}

// Called when the game starts or when spawned
void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (TrailNiagara)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrailNiagara valid: %s"), *TrailNiagara->GetName());
		TrailNiagara->Activate(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TrailNiagara is null!"));
	}

	
}

// Called every frame
void AArrowProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArrowProjectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		Destroy();
		return;
	}

	AController* MyOwnerInstigator = MyOwner->GetInstigatorController();
	UClass* DamageTypeClass = UDamageType::StaticClass();

	if (OtherActor && OtherActor != this && OtherActor != MyOwner)
	{	
		UE_LOG(LogTemp, Warning, TEXT("OnHit fired! OtherActor: %s"), *OtherActor->GetName());
		UGameplayStatics::ApplyDamage(OtherActor, Damage, MyOwnerInstigator, this, DamageTypeClass);
	}
	Destroy();
}

void AArrowProjectile::FireInDirection(const FVector& ShootDirection)
{
    ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
}

void AArrowProjectile::InitVelocity(float Speed)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->SetVelocityInLocalSpace(FVector::ForwardVector * Speed);
        ProjectileMovement->Activate();
    }
}

