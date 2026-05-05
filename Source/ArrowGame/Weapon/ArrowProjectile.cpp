// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../Character/ArcherCharacterBase.h"

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
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);
	
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

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		// 중요: 화살의 충돌 박스가 주인을 물리적으로 무시하도록 설정
		// 이렇게 하면 캐릭터 몸 안에서 생성되어도 밀어내지(튀어나가지) 않습니다.
		CollisionBox->IgnoreActorWhenMoving(MyOwner, true);
	}

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

	NotifyImpact(Hit);
	
	if (OtherActor->IsA(APawn::StaticClass())) 
	{
		if (HasAuthority()) // 서버에서만 처리
		{
			CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
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
}

void AArrowProjectile::NotifyImpact(const FHitResult& Hit)
{
	// 일반 화살은 여기서 아무것도 안 함! (ㅅㅂ 휴... 깨끗)
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

	if (TrailNiagara) TrailNiagara->Deactivate();

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void AArrowProjectile::HitPhysicsObject(UPrimitiveComponent* OtherComp, const FHitResult& Hit, AActor* MyOwner)
{
	// 1. 물리 체크 (입구 컷)
	if (!OtherComp || !OtherComp->IsSimulatingPhysics())
	{
		// 물리가 없는 물체라면 그냥 박기만 하고 종료
		StickIntoWorld(OtherComp, Hit.GetActor(), Hit);
		return;
	}

	// 2. 물리 충격 계산 및 적용
	FVector ImpulseDir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
	FVector Impulse = Damage * 100.0f * ImpulseDir;
    
	OtherComp->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);

	// 3. 충격 줬으니 이제 박히러 가자!
	StickIntoWorld(OtherComp, Hit.GetActor(), Hit);
}

void AArrowProjectile::StickIntoWorld(UPrimitiveComponent* OtherComp, AActor* OtherActor, const FHitResult& Hit)
{
	StopAndDisable();
	
	if (CollisionBox) 
	{
		CollisionBox->SetGenerateOverlapEvents(true);
		//라인트레이스를 위한
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
	
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
	
	if (OtherComp)
	{
		FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
		AttachToComponent(OtherComp, AttachRules);
	}
	UpdateOverlaps();
}

//나이아가라 서버호출
void AArrowProjectile::MulticastActivateTrail_Implementation()
{
	if (TrailNiagara)
	{
		TrailNiagara->Activate(true);
	}
}

void AArrowProjectile::PickUp(AArcherCharacterBase* Picker)
{
	// 박혀있을 때, 그리고 서버에서만 실행
	if (bStuck && HasAuthority() && Picker)
	{
		Picker->AddAmmo(ArrowType, 1); // 쏜 거 주웠으니 1개만 돌려줌
		Destroy();
	}
}