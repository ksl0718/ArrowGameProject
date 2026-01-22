// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "../Character/ArrowCharacter.h"
#include "ArrowProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"

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

    // ��¡ ����
    bIsCharging = true;
    ChargeTime = 0.f;
    BowState = EBowState::Charging;

    // ȭ�� ����
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // �浹 �����ϰ� ����

    PreparedArrow = GetWorld()->SpawnActor<AArrowProjectile>(ArrowProjectileClass, SpawnParams);
    if (!PreparedArrow)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: PreparedArrow spawn FAILED"));
        return;
    }

    // �߻��ڿ� �浹 ���� ����
    if (PreparedArrow->CollisionBox)
    {
        PreparedArrow->CollisionBox->IgnoreActorWhenMoving(OwnerCharacter, true);
        PreparedArrow->CollisionBox->MoveIgnoreActors.AddUnique(OwnerCharacter);
    }


    // �浹 + �̵� ��
    PreparedArrow->SetActorEnableCollision(false);
    if (PreparedArrow->GetProjectileMovement())
    {
        PreparedArrow->GetProjectileMovement()->StopMovementImmediately();
        PreparedArrow->GetProjectileMovement()->Deactivate();
    }

    // Arrow_Socket�� ����
    PreparedArrow->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Arrow_Socket")
    );
}

void ABow::EndDraw()
{
    if (!bIsCharging) return;

    bIsCharging = false;

    // PreparedArrow ���� ���� Ȯ��
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
	if (FireSound) // �߻� ���� ���
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            FireSound,
            GetActorLocation()
        );
    }

    // ���� ��ġ
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Bow: Mesh is NULL"));
		return;
	}

    FVector SpawnLoc = Mesh->GetSocketLocation(TEXT("Arrow_Socket"));
	FVector ShootDir = FVector::ZeroVector;
    APawn* PawnOwner = Cast<APawn>(OwnerCharacter);

    //�÷��̾� ĳ������ ��
    if (PawnOwner && PawnOwner->IsPlayerControlled())
    {
        UCameraComponent* CameraComp = OwnerCharacter->FindComponentByClass<UCameraComponent>();

        if (!CameraComp) return;

        FVector TraceStart = CameraComp->GetComponentLocation();
        FVector TraceDir = CameraComp->GetForwardVector();

        FVector TraceEnd = TraceStart + TraceDir * 10000.f;

        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(BowFireTrace), false);

        Params.AddIgnoredActor(OwnerCharacter);
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(PreparedArrow);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, TraceStart, TraceEnd, ECC_Visibility, Params
        );

        FVector TargetPoint = bHit ? Hit.ImpactPoint : TraceEnd; // ���� ���� �Ǵ� �ִ� �Ÿ�

        ShootDir = (TargetPoint - SpawnLoc).GetSafeNormal(); // �߻� ����
    }
	else { //AI ĳ������ ��
        ShootDir = OwnerCharacter->GetActorForwardVector();
    }
	

    float Speed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, ChargePercent);
    FVector ShootVelocity = ShootDir * Speed;

    // 1) �տ��� ����
    if (OwnerCharacter && OwnerCharacter->FireMontage) {
        OwnerCharacter->PlayMontage(OwnerCharacter->FireMontage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter is null in Bow::FireArrow"));
    }

    PreparedArrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (PreparedArrow->TrailNiagara)
    {
        PreparedArrow->TrailNiagara->Activate(true);
    }
    PreparedArrow->SetActorRotation(ShootDir.Rotation());

    float HalfLength = 0.f;
    if (PreparedArrow->CollisionBox)
    {
        // X���� ���� �����̶�� ���� (BoxExtent 40,2,2 �̴ϱ�)
        HalfLength = PreparedArrow->CollisionBox->GetScaledBoxExtent().X;
    }

    const float ExtraMargin = 10.f;          // ��¦ �� ��
    const float StartOffset = HalfLength + ExtraMargin;

    // Arrow_Socket ���� ��ġ���� ShootDir �������� �� ����
    FVector NewLoc = SpawnLoc + ShootDir * StartOffset;
    PreparedArrow->SetActorLocation(NewLoc);

    // 3) ���� �浹 �Ѱ� �߻�
    PreparedArrow->SetActorEnableCollision(true);

    if (auto Move = PreparedArrow->GetProjectileMovement())
    {
        Move->Velocity = ShootVelocity;
        Move->Activate();
    }


    PreparedArrow = nullptr;
}