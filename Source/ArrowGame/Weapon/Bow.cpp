// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "../Character/ArrowCharacter.h"
#include "ArrowProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 사용 위함
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ABow::ABow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ABow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABow, BowState);
    DOREPLIFETIME(ABow, bIsCharging);
    DOREPLIFETIME(ABow, ChargeTime);
    DOREPLIFETIME(ABow, bIsAiming);
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

void ABow::ServerStartDraw_Implementation()
{
    UE_LOG(LogTemp, Error, TEXT("Server OK"));
    
    bIsCharging = true;  // 이 값이 변경되면
    ChargeTime = 0.f;
    BowState = EBowState::Charging;
    
    OnRep_IsCharging();
}

void ABow::OnRep_IsCharging()
{
    if (bIsCharging)
    {
        // 활 당기기 시작 → 화살 생성
        SpawnDrawArrow();
    }
    else
    {
        // 활 놓기 → 화살 제거
        DestroyDrawArrow();
    }
}


void ABow::StartDraw()
{
    if (!bIsAiming) return;
    if (!OwnerCharacter) return;
    if (!ArrowProjectileClass) return;
    
    if (DrawSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            DrawSound,
            GetActorLocation()
        );
    }
    
    SpawnDrawArrow();
    
    if (HasAuthority())  // 서버라면
    {
        bIsCharging = true;
        ChargeTime = 0.f;
        BowState = EBowState::Charging;
        
        // 서버는 RepNotify가 자동 호출 안 되므로 수동 호출
        OnRep_IsCharging();
    }
    else  // 클라이언트라면
    {
        ServerStartDraw();  // 서버에 요청
    }
}

void ABow::SpawnDrawArrow()
{
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh || !Mesh->DoesSocketExist(TEXT("Arrow_Socket")))
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter;

    PreparedArrow = GetWorld()->SpawnActor<AArrowProjectile>(
        ArrowProjectileClass, 
        SpawnParams
    );
    
    if (!PreparedArrow)
    {
        return;
    }

    // 복제 안 되는 로컬 액터로 설정
    PreparedArrow->SetReplicates(false);
    PreparedArrow->SetActorEnableCollision(false);

    if (PreparedArrow->GetProjectileMovement())
    {
        PreparedArrow->GetProjectileMovement()->StopMovementImmediately();
        PreparedArrow->GetProjectileMovement()->Deactivate();
    }

    PreparedArrow->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Arrow_Socket")
    );
}

void ABow::DestroyDrawArrow()
{
    if (IsValid(PreparedArrow))
    {
        PreparedArrow->Destroy();
        PreparedArrow = nullptr;
    }
}

void ABow::ServerEndDraw_Implementation()
{
    if (!bIsCharging) return;

    float ChargePercent = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);
    
    // 먼저 발사 처리 (PreparedArrow는 사용 안 함)
    FireArrow(ChargePercent);
    
    // 그 다음 상태 변경 (장전 화살 삭제)
    bIsCharging = false;
    ChargeTime = 0.f;
    BowState = EBowState::Idle;
    
    // 서버는 수동으로 호출
    OnRep_IsCharging();
}

void ABow::EndDraw()
{
    if (!bIsCharging) return;
    
    // PreparedArrow 체크 제거 (더 이상 필요 없음)
    ServerEndDraw();
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
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: OwnerCharacter is NULL"));
        return;
    }
    
    if (FireSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(this, FireSound, GetActorLocation());
    }

    // 메시 체크
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: Mesh is NULL"));
        return;
    }

    // ========== 발사 방향 계산 ==========
    
    FVector SocketLoc = Mesh->GetSocketLocation(TEXT("Arrow_Socket"));
    
    FVector ShootDir;
    AController* Controller = OwnerCharacter->GetController();
    if (Controller)
    {
        FVector CamLoc;
        FRotator CamRot;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
        // 카메라가 보는 방향으로 100m 레이저 발사
        FVector TraceEnd = CamLoc + (CamRot.Vector() * 10000.f);
        
        FHitResult AimHit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);
        
        // 카메라 시선 검사
        if (GetWorld()->LineTraceSingleByChannel(AimHit, CamLoc, TraceEnd, ECC_Visibility, Params))
        {
            // 무언가 보고 있다면 그 지점을 향해 쏜다
            ShootDir = (AimHit.Location - SocketLoc).GetSafeNormal();
        }
        else
        {
            // 허공이라면 100m 앞을 향해 쏜다
            ShootDir = (TraceEnd - SocketLoc).GetSafeNormal();
        }
    }else
    {
        // AI 등의 경우 소켓 방향 사용
        ShootDir = Mesh->GetSocketRotation(TEXT("Arrow_Socket")).Vector();
    }
    
    const float IdealDistance = 10.f; 
    FVector FinalSpawnLoc = SocketLoc + (ShootDir * IdealDistance);
    
    FHitResult WallHit;
    FCollisionQueryParams WallParams;
    WallParams.AddIgnoredActor(OwnerCharacter);
    
    // 소켓 ~ 목표위치 사이에 벽이 있는지 검사
    if (GetWorld()->LineTraceSingleByChannel(WallHit, SocketLoc, FinalSpawnLoc, ECC_Visibility, WallParams))
    {
        // 벽이 있으면 벽 바로 앞(1cm)으로 당김
        FinalSpawnLoc = WallHit.Location - (ShootDir * 1.0f);
    }

    // 4. [화살 생성] Instigator 설정 필수
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter; 
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    UE_LOG(LogTemp, Warning, TEXT("================== [FIRE ARROW DEBUG] =================="));

    // 1. 소켓 위치와 최종 스폰 위치 비교
    float DistFromSocket = FVector::Dist(SocketLoc, FinalSpawnLoc);
    UE_LOG(LogTemp, Warning, TEXT("1. Spawn Distance from Socket: %f"), DistFromSocket);

    // 2. 캐릭터와의 거리 확인 (캡슐 반경보다 커야 안전)
    if (OwnerCharacter && OwnerCharacter->GetCapsuleComponent())
    {
        float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
        float DistFromCenter = FVector::Dist(OwnerCharacter->GetActorLocation(), FinalSpawnLoc);
    
        UE_LOG(LogTemp, Warning, TEXT("2. Capsule Radius: %f / Dist From Center: %f"), CapsuleRadius, DistFromCenter);

        if (DistFromCenter <= CapsuleRadius)
        {
            UE_LOG(LogTemp, Error, TEXT("🚨 DANGER! Spawning INSIDE Capsule! (Character will be pushed)"));
            // 시각적으로 확인하기 위해 빨간 구체 그리기
            DrawDebugSphere(GetWorld(), FinalSpawnLoc, 10.f, 12, FColor::Red, false, 3.f);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✅ SAFE. Spawning OUTSIDE Capsule."));
            // 안전하면 초록 구체
            DrawDebugSphere(GetWorld(), FinalSpawnLoc, 10.f, 12, FColor::Green, false, 3.f);
        }
    }
    
    // 회전값: ShootDir(조준방향)을 그대로 사용 -> ArrowProjectile에서 Velocity가 0이어도 이 회전값을 씀
    AArrowProjectile* FiredArrow = GetWorld()->SpawnActor<AArrowProjectile>(
        ArrowProjectileClass,
        FinalSpawnLoc,
        ShootDir.Rotation(), 
        SpawnParams
    );
    
    if (!FiredArrow)
    {
        return;
    }

    // 네트워크 및 물리 설정
    FiredArrow->SetReplicates(true); 
    FiredArrow->SetReplicateMovement(true);

    //혹시 모를 충돌 방지를 위해 Owner 무시 설정
    if (FiredArrow->CollisionBox)
    {
        FiredArrow->CollisionBox->IgnoreActorWhenMoving(OwnerCharacter, true);
        FiredArrow->CollisionBox->MoveIgnoreActors.AddUnique(OwnerCharacter);
    }

    // 이펙트 활성화
    if (FiredArrow->TrailNiagara)
    {
        FiredArrow->TrailNiagara->Activate(true);
    }

    // 발사 속도 적용
    float Speed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, ChargePercent);
    FVector FinalVelocity = ShootDir * Speed;
    
    FiredArrow->InitVelocity(FinalVelocity);
    
    if (auto MoveComp = FiredArrow->GetProjectileMovement())
    {
        MoveComp->Velocity = ShootDir * Speed;
        MoveComp->Activate();
    }
}

void ABow::SetAiming(bool bNewAiming)
{
    // 내가 서버라면? 그냥 바꿈
    if (HasAuthority())
    {
        bIsAiming = bNewAiming;
    }
    // 내가 클라이언트라면? 서버한테 부탁함 (RPC)
    else
    {
        ServerSetAiming(bNewAiming);
    }
}

void ABow::ServerSetAiming_Implementation(bool bNewAiming)
{
    // 여기서 바꾸면 -> DOREPLIFETIME 설정 때문에 -> 모든 클라이언트로 자동 전파됨
    bIsAiming = bNewAiming;
}
