// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "ArrowCharacter.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AArrowProjectile::AArrowProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	CollisionBox->SetBoxExtent(FVector(40.f, 2.f, 2.f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetSimulatePhysics(false);

	CollisionBox->SetNotifyRigidBodyCollision(true);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(CollisionBox);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetRelativeScale3D(FVector(2.7f, 2.7f, 2.7f));
    ArrowMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));


	TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	TrailNiagara->SetupAttachment(RootComponent);
	TrailNiagara->bAutoActivate = false;


    // 충돌 이벤트 연결
	CollisionBox->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnHit);

    // 움직임 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 6000.f;
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TrailNiagara is null!"));
	}

	if (APawn* InstPawn = GetInstigator())
	{
		CollisionBox->IgnoreActorWhenMoving(InstPawn, true);
		CollisionBox->MoveIgnoreActors.AddUnique(InstPawn);
	}

	if (AActor* OwnerActor = GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(OwnerActor, true);
		CollisionBox->MoveIgnoreActors.AddUnique(OwnerActor);
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
	if (bStuck) // 이미 박혔으면 무시
		return;

	if (!OtherActor || OtherActor == this) // 유효하지 않은 액터면 무시
		return;

	if (OtherActor == GetInstigator() || OtherActor == GetOwner()) // 발사자면 무시
		return;

	UE_LOG(LogTemp, Warning, TEXT("HIT: %s"), *OtherActor->GetName());
	
	AActor* MyOwner = GetOwner();
	if (!MyOwner)
	{
		Destroy();
		return;
	}

	if (OtherActor != MyOwner) {
		AController* MyOwnerInstigator = MyOwner->GetInstigatorController();
		UClass* DamageTypeClass = UDamageType::StaticClass();
		
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			MyOwnerInstigator,
			this,
			DamageTypeClass
		);
	}

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		StopAndDisable();
		Destroy();
		//// 스켈메시 찾기
		//USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(OtherComp);
		//if (!SkelComp)
		//{
		//	SkelComp = HitPawn->FindComponentByClass<USkeletalMeshComponent>();
		//	UE_LOG(LogTemp, Warning, TEXT("No Fount SkelComp"));
		//}

		//if (SkelComp)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("Fount SkelComp"));
		//	FVector BoneWorldPos;
		//	FName ClosestBone = SkelComp->FindClosestBone(
		//		Hit.ImpactPoint,
		//		&BoneWorldPos,
		//		0.f,
		//		false
		//	);
		//	// 본에 붙이기
		//	AttachToComponent(
		//		SkelComp,
		//		FAttachmentTransformRules::KeepWorldTransform,
		//		(ClosestBone != NAME_None) ? ClosestBone : NAME_None
		//	);
		//	UE_LOG(LogTemp, Warning, TEXT("BoneName: %s"), *ClosestBone.ToString());
		//}
		//else {
		//	UE_LOG(LogTemp, Warning, TEXT("still Fount SkelComp"));
		//	AttachToActor(
		//		OtherActor,
		//		FAttachmentTransformRules::KeepWorldTransform
		//	);
		//}
		return;
	}
	else if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		HitPhysicsObject(OtherComp, Hit, MyOwner);
		return;
	}
	else
	{
		StickIntoWorld(OtherComp, OtherActor, Hit);
	}
}


void AArrowProjectile::FireInDirection(const FVector& ShootDirection)
{
    ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
}

void AArrowProjectile::InitVelocity(const FVector& Velocity)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Velocity;
		ProjectileMovement->Activate();
	}
}

void AArrowProjectile::StopAndDisable()
{
	bStuck = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (TrailNiagara)
	{
		TrailNiagara->Deactivate();
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetSimulatePhysics(false);
	}
	SetActorEnableCollision(false);
}

void AArrowProjectile::HitPhysicsObject(UPrimitiveComponent* OtherComp, const FHitResult& Hit, AActor* MyOwner)
{
	if (!OtherComp || !OtherComp->IsSimulatingPhysics())
		return;
	const float ImpulseStrength = Damage * 100.f;

	//임펄스 추가
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		// 화살이 날아가던 방향 기준으로
		FVector ImpulseDir;

		if (ProjectileMovement)
		{
			ImpulseDir = ProjectileMovement->Velocity.GetSafeNormal();
		}
		else
		{
			ImpulseDir = GetActorForwardVector();
		}

		FVector	Impulse = Damage * 100.0f * ImpulseDir;  // 거리비례 임펄스 약해짐
		OtherComp->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);
	}
	//화살 박히기
	StopAndDisable();
	StickIntoWorld(OtherComp, Hit.GetActor(), Hit);
}

void AArrowProjectile::StickIntoWorld(UPrimitiveComponent* OtherComp, AActor* OtherActor, const FHitResult& Hit)
{
	StopAndDisable();

	// 화살 방향(회전) 맞추기
	FVector ForwardDir;
	if (ProjectileMovement && !ProjectileMovement->Velocity.IsNearlyZero())
	{
		ForwardDir = ProjectileMovement->Velocity.GetSafeNormal();
	}
	else
	{
		ForwardDir = GetActorForwardVector();
	}
	SetActorRotation(ForwardDir.Rotation());

	// 위치: 충돌 지점에서 화살 반 길이만큼 안쪽으로
	float HalfLength = 40.f;
	if (CollisionBox)
	{
		HalfLength = CollisionBox->GetScaledBoxExtent().X;
	}

	const FVector NewLocation = Hit.ImpactPoint - ForwardDir * HalfLength * 0.8f;
	SetActorLocation(NewLocation);

	if (OtherComp)
	{
		AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
	}
}