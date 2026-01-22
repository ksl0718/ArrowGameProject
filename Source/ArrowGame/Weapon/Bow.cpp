// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "../Character/ArrowCharacter.h"
#include "ArrowProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 매크로 사용해야 함
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

/**클라이언트가 호출하는 함수 (입력 -> 서버 요청)*/
void ABow::StartDraw()
{

    // 소리는 내 컴퓨터(클라이언트)에서 즉시 재생
    if (DrawSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            DrawSound,
            GetActorLocation()
        );
    }
    
    ServerStartDraw();
}

void ABow::ServerStartDraw_Implementation()
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

    // Arrow_Socket�� ���� (Replicates 했으니 아마 클라이언트에서도 붙은 모습이 보일거..)
    PreparedArrow->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Arrow_Socket")
    );
}

void ABow::EndDraw()
{
    // 서버에 발사 요청
    ServerEndDraw();
    
    
}

void ABow::ServerEndDraw_Implementation()
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

    // 서버에서 발사 함수 실행
    FireArrow(ChargePercent);
    BowState = EBowState::Idle;
}

void ABow::HandleCharge(float DeltaTime)
{
    ChargeTime += DeltaTime;

    if (ChargeTime >= AutoReleaseTime)
    {
        // 서버에서만 오토리리즈 체크하고 싶다면 HasAuthority() 체크
        // 여기서는 편의상 그대로 둠 (EndDraw 호출 시 RPC로 서버에 감)
        UE_LOG(LogTemp, Warning, TEXT("Bow: Auto release"));
        EndDraw();
    }
}

void ABow::FireArrow(float ChargePercent)
{
    // 이 함수는 ServerEndDraw를 통해 호출되므로 서버에서 실행됩니다.

    if (!PreparedArrow || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: FireArrow called but missing reference."));
        return;
    }

    // 발사 사운드 (서버에서 재생 -> Replicated Sound 혹은 Multicast 필요할 수 있음)
    // 지금은 서버 플레이어만 듣거나, Listen Server인 경우 호스트가 들을듯
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

#pragma region 멀티 관련 여기 넣겠음

/** 어떤 변수를 동기화할지 등록*/
void ABow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // PreparedArrow, Charging, State 변수가 서버에서 변하면 클라이언트에게 알랴줌
    DOREPLIFETIME(ABow, PreparedArrow);
    DOREPLIFETIME(ABow, bIsCharging);
    DOREPLIFETIME(ABow, BowState);
}

/**DOREPLIFETIME(ClassName, VarName): 해당 변수를 네트워크 복제 대상으로 등록합니다. 가장 많이 사용됩니다.

DOREPLIFETIME_CONDITION(ClassName, VarName, Condition): 특정 조건이 만족될 때만 복제하여 네트워크 대역폭을 절약합니다.

COND_OwnerOnly: 이 액터를 소유한 클라이언트에게만 전송 (예: 인벤토리 데이터).

COND_SkipOwner: 소유자를 제외한 나머지 사람들에게 전송 (예: 3인칭 애니메이션 관련 변수).*/

#pragma endregion 