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

	AActor* InstigatorActor = GetInstigator(); 
	if (OtherActor == InstigatorActor) // 발사자 자신이면 무시
		return;

	UE_LOG(LogTemp, Warning, TEXT("HIT: %s"), *OtherActor->GetName());

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
	
	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (HitPawn)
	{
		// 0) 공통 종료 처리 플래그
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
		SetActorEnableCollision(false);   // 박힌 후엔 완전 비충돌

		// 1) 스켈메시 찾기
		USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(OtherComp);
		if (!SkelComp)
		{
			SkelComp = HitPawn->FindComponentByClass<USkeletalMeshComponent>();
		}

		if (SkelComp)
		{
			// 2) 가장 가까운 본 찾기
			FVector BoneWorldPos;
			FName ClosestBone = SkelComp->FindClosestBone(
				Hit.ImpactPoint,
				&BoneWorldPos,     // 위치도 받아둠 (안 써도 됨)
				0.f,
				false
			);

			// 3) 화살 방향(회전) 맞추기
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

			// 4) 위치: 충돌 지점에서 화살 반 길이만큼 뒤로 빼기
			float HalfLength = 40.f;
			if (CollisionBox)
			{
				HalfLength = CollisionBox->GetScaledBoxExtent().X; // X축이 길이 방향이라고 가정
			}

			// ImpactPoint 기준으로 몸 안쪽으로 조금 밀어넣기
			const FVector NewLocation = Hit.ImpactPoint - ForwardDir * HalfLength * 0.8f;
			SetActorLocation(NewLocation);

			// 5) 본에 붙이기 (위치/회전은 지금 세팅한 월드값 그대로 유지)
			AttachToComponent(
				SkelComp,
				FAttachmentTransformRules::KeepWorldTransform,
				ClosestBone != NAME_None ? ClosestBone : NAME_None
			);
		}
		else
		{
			// 스켈메시 못 찾으면 그냥 액터에 붙이기
			AttachToActor(
				OtherActor,
				FAttachmentTransformRules::KeepWorldTransform
			);
		}

		return; // 캐릭터 분기 끝, 밑에 임펄스/월드 박힘 로직 안 타게
	}

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
	/// 박히는 효과 처리
	bStuck = true;

	if (ProjectileMovement) {
		ProjectileMovement->StopMovementImmediately(); // 움직임 멈춤
		ProjectileMovement->Deactivate(); // 무브먼트 비활성화
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetSimulatePhysics(false);
	}
	SetActorEnableCollision(false);; // 물리 시뮬레이션 비활성화

	if (TrailNiagara) {
		TrailNiagara->Deactivate(); // 이펙트 비활성화
	}

	//const FRotator ArrowRotation = (-Hit.ImpactNormal).Rotation(); // 화살이 박힌 방향으로 회전
	//SetActorRotation(ArrowRotation); // 화살 회전 설정

	if (OtherComp) {
		AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform); // 충돌한 컴포넌트에 부착
	}
	else {
		AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform); // 충돌한 액터에 부착
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

