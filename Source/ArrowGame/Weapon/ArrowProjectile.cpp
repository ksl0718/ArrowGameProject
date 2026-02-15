// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "../Character/ArrowCharacter.h"
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
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	CollisionBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	if (ArrowMesh)
	{
		ArrowMesh->SetCollisionProfileName(TEXT("NoCollision"));
		ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	CollisionBox->SetNotifyRigidBodyCollision(false);	
	CollisionBox->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnHit);
	
	//Mesh
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(CollisionBox);
	ArrowMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//ArrowMesh->SetRelativeScale3D(FVector(2.7f, 2.7f, 2.7f));
    //ArrowMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	
	//Trail
	TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	TrailNiagara->SetupAttachment(RootComponent);
	TrailNiagara->bAutoActivate = false;
	

    //ProjectileMovement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 6000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.5f; // ȭ�� ������
	
	// 정확도 보강 (중요)
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->MaxSimulationTimeStep = 0.02f;
	ProjectileMovement->MaxSimulationIterations = 8;
	
	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	FString RoleStr = HasAuthority() ? TEXT("Server") : TEXT("Client");
	UE_LOG(LogTemp, Warning, TEXT("=== ARROW BORN [%s] ==="), *RoleStr);

	// 1. Instigator(주인) 확인
	AActor* MyOwner = GetInstigator();
	UE_LOG(LogTemp, Warning, TEXT("1. Instigator: %s"), MyOwner ? *MyOwner->GetName() : TEXT("NULL (Problem!)"));

	// 2. CollisionBox 상태 확인
	if (CollisionBox)
	{
		ECollisionResponse Resp = CollisionBox->GetCollisionResponseToChannel(ECC_Pawn);
		FString RespStr = UEnum::GetValueAsString(Resp);
		FString Profile = CollisionBox->GetCollisionProfileName().ToString();

		UE_LOG(LogTemp, Warning, TEXT("2. [BOX] Profile: %s | Response to Pawn: %s"), *Profile, *RespStr);
        
		if (Resp == ECR_Block)
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 BOX IS BLOCKING PAWN! (This is the culprit)"));
		}
	}

	// 3. ArrowMesh 상태 확인 (여기가 제일 의심됨)
	if (ArrowMesh)
	{
		ECollisionResponse Resp = ArrowMesh->GetCollisionResponseToChannel(ECC_Pawn);
		FString RespStr = UEnum::GetValueAsString(Resp);
		FString Profile = ArrowMesh->GetCollisionProfileName().ToString();

		UE_LOG(LogTemp, Warning, TEXT("3. [MESH] Profile: %s | Response to Pawn: %s"), *Profile, *RespStr);

		if (Resp == ECR_Block)
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 MESH IS BLOCKING PAWN! (You forgot to set NoCollision)"));
		}
	}
	
	PrevLocation = GetActorLocation();
	
	if (TrailNiagara)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrailNiagara valid: %s"), *TrailNiagara->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TrailNiagara is null!"));
	}
	// CollisionBox 체크 추가!
	if (!CollisionBox)
	{
		UE_LOG(LogTemp, Error, TEXT("ArrowProjectile: CollisionBox is NULL!"));
		return;
	}
	
	if (APawn* InstPawn = GetInstigator())
	{
		CollisionBox->IgnoreActorWhenMoving(InstPawn, true);
		CollisionBox->MoveIgnoreActors.AddUnique(InstPawn);
		UE_LOG(LogTemp, Log, TEXT("Arrow ignoring Instigator: %s"), *InstPawn->GetName());
	}

	if (AActor* OwnerActor = GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(OwnerActor, true);
		CollisionBox->MoveIgnoreActors.AddUnique(OwnerActor);
		UE_LOG(LogTemp, Log, TEXT("Arrow ignoring Owner: %s"), *OwnerActor->GetName());
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
	UE_LOG(LogTemp, Warning, TEXT("Arrow Hit: %s"), *OtherActor->GetName());
	
	if (bStuck) // �̹� �������� ����
		return;

	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator()) return;

	if (OtherActor->IsA(APawn::StaticClass())) 
	{
		if (HasAuthority()) // 서버에서만 처리
		{
			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);

			// [핵심] 맞자마자 화살 삭제! (박히는 로직 안 탐)
			Destroy(); 
		}
		else 
		{
			// 클라이언트에서는 즉시 숨겨서 반응성을 높임 (선택사항)
			SetActorHiddenInGame(true);
		}
        
		// 여기서 함수 종료 (아래 꽂히는 로직 실행 안 함)
		return; 
	}
	
	HitPhysicsObject(OtherComp, Hit, GetOwner());
	
	StickIntoWorld(OtherComp, OtherActor, Hit);
	
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

	//if (CollisionBox)
	//{
		//CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		//CollisionBox->SetSimulatePhysics(false);
	//}
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
}

void AArrowProjectile::HitPhysicsObject(UPrimitiveComponent* OtherComp, const FHitResult& Hit, AActor* MyOwner)
{
	if (!OtherComp || !OtherComp->IsSimulatingPhysics())
		return;
	const float ImpulseStrength = Damage * 100.f;

	//���޽� �߰�
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		// ȭ���� ���ư��� ���� ��������
		FVector ImpulseDir;

		if (ProjectileMovement)
		{
			ImpulseDir = ProjectileMovement->Velocity.GetSafeNormal();
		}
		else
		{
			ImpulseDir = GetActorForwardVector();
		}

		FVector	Impulse = Damage * 100.0f * ImpulseDir;  // �Ÿ���� ���޽� ������
		OtherComp->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);
	}
	//ȭ�� ������
	StopAndDisable();
	StickIntoWorld(OtherComp, Hit.GetActor(), Hit);
}

void AArrowProjectile::StickIntoWorld(UPrimitiveComponent* OtherComp, AActor* OtherActor, const FHitResult& Hit)
{
	StopAndDisable();

	// ȭ�� ����(ȸ��) ���߱�
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

	//float HalfLen = CollisionBox->GetScaledBoxExtent().X;
	//FVector NewLoc = Hit.ImpactPoint - ForwardDir * HalfLen;

	//SetActorLocation(NewLoc);

	if (OtherComp)
	{
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepWorld, 
			EAttachmentRule::KeepWorld, 
			EAttachmentRule::KeepWorld, 
			false
		);
		AttachToComponent(OtherComp, AttachRules);
	}
}