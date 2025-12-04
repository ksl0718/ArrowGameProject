// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "ArrowCharacter.h"
#include "ArrowProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
ABow::ABow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCharging)
	{
		HandleCharge(DeltaTime);
	}

}

void ABow::StartAim()
{
    if (!bIsCharging)
    {
        bIsAiming = true;
        BowState = EBowState::Aim;
    }

}

void ABow::StopAim()
{
    if (!bIsCharging)
    {
        bIsAiming = false;
        BowState = EBowState::Idle;
    }
}

void ABow::StartDraw()
{
    if (!bIsAiming) return;
    if (!OwnerCharacter) return;
    if (!ArrowProjectileClass) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: Mesh is NULL"));
        return;
    }

    if (!Mesh->DoesSocketExist(TEXT("Arrow_Socket")))
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: Arrow_Socket NOT FOUND on mesh"));
        return;
    }

    if (DrawSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            DrawSound,
            GetActorLocation()
        );
    }

    // 차징 시작
    bIsCharging = true;
    ChargeTime = 0.f;
    BowState = EBowState::Charging;

    // 화살 스폰
    PreparedArrow = GetWorld()->SpawnActor<AArrowProjectile>(ArrowProjectileClass);
    if (!PreparedArrow)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: PreparedArrow spawn FAILED"));
        return;
    }

    PreparedArrow->SetOwner(OwnerCharacter);
    PreparedArrow->CollisionBox->IgnoreActorWhenMoving(OwnerCharacter, true);

    // 충돌 + 이동 끔
    PreparedArrow->SetActorEnableCollision(false);
    if (PreparedArrow->GetProjectileMovement())
    {
        PreparedArrow->GetProjectileMovement()->StopMovementImmediately();
        PreparedArrow->GetProjectileMovement()->Deactivate();
    }

    // Arrow_Socket에 부착
    PreparedArrow->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Arrow_socket")
    );
}

void ABow::EndDraw()
{
    if (!bIsCharging) return;

    bIsCharging = false;

    // PreparedArrow 존재 여부 확인
    if (!PreparedArrow)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: EndDraw called but PreparedArrow NULL"));
        return;
    }

    float ChargePercent = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);
    ChargeTime = 0.f;

    FireArrow(ChargePercent);
    BowState = EBowState::Idle;
}

void ABow::HandleCharge(float DeltaTime)
{
    ChargeTime += DeltaTime;

    if (ChargeTime >= AutoReleaseTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bow: Auto release"));
        EndDraw();
    }
}

void ABow::FireArrow(float ChargePercent)
{
    if (!PreparedArrow || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: FireArrow called but missing reference."));
        return;
    }
    if (FireSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            FireSound,
            GetActorLocation()
        );
    }
    // 스폰 위치
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    FVector SpawnLoc = Mesh->GetSocketLocation(TEXT("Arrow_Socket"));

    // 카메라 방향 추적
    FVector CamLoc;
    FRotator CamRot;
    OwnerCharacter->GetController()->GetPlayerViewPoint(CamLoc, CamRot);

    FVector TraceStart = CamLoc;
    FVector TraceEnd = TraceStart + CamRot.Vector() * 10000.f;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, TraceStart, TraceEnd, ECC_Visibility, Params
    );

    FVector TargetPoint = bHit ? Hit.ImpactPoint : TraceEnd;
    FVector ShootDir = (TargetPoint - SpawnLoc).GetSafeNormal();

    float Speed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, ChargePercent);
    FVector ShootVelocity = ShootDir * Speed;

    // 1) 손에서 떼기
    if (OwnerCharacter) {
        OwnerCharacter->PlayFireMontage();
    }
    PreparedArrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (PreparedArrow->TrailNiagara)
    {
        PreparedArrow->TrailNiagara->Activate(true);
    }
    // 2) 충돌/이동 활성화
    PreparedArrow->SetActorEnableCollision(true);
    PreparedArrow->GetProjectileMovement()->Activate();

    PreparedArrow->CollisionBox->IgnoreActorWhenMoving(OwnerCharacter, true);

    if (PreparedArrow->GetProjectileMovement())
    {
        PreparedArrow->GetProjectileMovement()->Activate();
        PreparedArrow->GetProjectileMovement()->Velocity = ShootVelocity;
    }

    PreparedArrow = nullptr;
}