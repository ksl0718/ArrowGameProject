// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "../Character/ArcherCharacterBase.h"
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
    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
}

void ABow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABow, BowState);
    DOREPLIFETIME(ABow, bIsCharging);
    DOREPLIFETIME(ABow, bIsVisualAiming);
    DOREPLIFETIME(ABow, bIsReloading);
    DOREPLIFETIME(ABow, ChargeTime);
    DOREPLIFETIME(ABow, bIsNocking);
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
    if (OwnerArcherCharacter == nullptr) return;
	if (bIsCharging)
	{
	    if (!OwnerArcherCharacter-> IsAiming()) // 조준 상태가 아닌 경우 drawing 상태 변경
	    {
	        bIsCharging = false;
	        return;
	    }
	    HandleCharge(DeltaTime);
	}
}

void ABow::StartAim()
{
    if (OwnerArcherCharacter && !OwnerArcherCharacter->IsAiming()) 
    {
        UE_LOG(LogTemp, Error, TEXT("StartAim Blocked: Player is NOT holding the button!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Start BowAim"));
    // 이미 조준 중이거나 재장전(발사 후 쿨타임) 중이면 무시
    if (bIsNocking || bIsReloading || bIsCharging) return;

    if (HasAuthority())
    {
        bIsVisualAiming = true;
        bIsNocking = true;
        BowState = EBowState::Aim;
        UpdateArrowVisual();
    }
    else
    {
        // 클라이언트라면 서버에 조준 시작을 알림
        ServerStartAim();
        // 내 화면에선 즉시 시각화 (예측)
        bIsVisualAiming = true;
        UpdateArrowVisual();
    }
    
    GetWorldTimerManager().SetTimer(NockingTimerHandle, this, &ABow::FinishNocking, NockingDelayTime, false);
    
}

void ABow::StopAim()
{
    CancelAction();
}
void ABow::ServerStartAim_Implementation()
{
    StartAim(); // 서버에서도 똑같이 실행하여 변수 복제 유도
}

void ABow::ServerStartDraw_Implementation()
{
    UE_LOG(LogTemp, Error, TEXT("Server OK"));
    
    bIsCharging = true;  // 이 값이 변경되면
    ChargeTime = 0.f;
    BowState = EBowState::Charging;
    
    OnRep_IsCharging();
}
// 에임 상태가 변할 때 호출 (서버가 값을 바꾸면 모든 클라이언트에서 실행됨)
void ABow::OnRep_IsVisualAiming()
{
    UE_LOG(LogTemp, Error, TEXT("OnRep_IsVisualAiming"));
    UpdateArrowVisual();
}

void ABow::OnRep_IsCharging()
{
    UpdateArrowVisual();
}

void ABow::OnRep_IsReloading()
{
    UE_LOG(LogTemp, Error, TEXT("OnRep_IsReloading"));
    if (!bIsReloading)
    {
        // 내 화면에서도 화살 가시성을 업데이트
        UpdateArrowVisual();
    }
}

void ABow::OnRep_IsNocking()
{
    UE_LOG(LogTemp, Error, TEXT("OnRep_IsNocking"));
    // 장전 시작/끝 시점에 시각적 효과가 필요하다면 여기서 처리
    UpdateArrowVisual(); 
}

void ABow::UpdateArrowVisual()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateArrowVisual"));
    // 조건이 하나라도 안 맞으면 무조건 지운다는 마인드
    if (!OwnerArcherCharacter) return;
    

    // 2. [핵심] 화살이 보여야 하는 '모든' 상황을 정의합니다.
    // (장전 시각화 중이거나 OR 당기는 중일 때) AND (재장전 중이 아닐 때) AND (캐릭터가 조준 중일 때)
    bool bShouldShow = (bIsVisualAiming || bIsCharging) && !bIsReloading;

    if (bShouldShow)
    {
        UE_LOG(LogTemp, Warning, TEXT("Should Show"));
        // 보여줘야 하는 상황이면 Spawn 시도
        if (PreparedArrow == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("=== ARROW SPAWN === (Visual:%d, Charging:%d)"), bIsVisualAiming, bIsCharging);
            SpawnDrawArrow();
        }
        UE_LOG(LogTemp, Warning, TEXT("PreparedArrow"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("else"));
        // [이분법] 보여줄 상황이 '조금이라도' 아니라면, 이유 불문하고 무조건 Destroy
        if (PreparedArrow)
        {
            UE_LOG(LogTemp, Error, TEXT("=== ARROW DESTROY === (Visual:%d, Charging:%d)"), bIsVisualAiming, bIsCharging);
            DestroyDrawArrow();
            PreparedArrow = nullptr; // 포인터 초기화 필수
        }
        UE_LOG(LogTemp, Warning, TEXT("not PreparedArrow"));
    }
}

void ABow::StartDraw()
{
    if (bIsReloading || !OwnerArcherCharacter->IsAiming()) return;

    if (bIsNocking)
    {
        bPendingDraw = true;
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
    
    if (HasAuthority())
    {
        bIsCharging = true;
        ChargeTime = 0.f;
        BowState = EBowState::Charging;
        UpdateArrowVisual(); // 서버 본인 업데이트
    }
    else
    {
        // 클라이언트는 변수를 예측으로 먼저 바꾸고 업데이트
        ServerStartDraw();
        bIsCharging = true; 
        UpdateArrowVisual(); 
        
    }
}

void ABow::SpawnDrawArrow()
{
    if (PreparedArrow && PreparedArrow->IsValidLowLevel()) 
    {
        return; 
    }
    
    USkeletalMeshComponent* CharacterMesh = OwnerArcherCharacter ? OwnerArcherCharacter->GetMesh() : nullptr;
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bow: Character mesh missing."));
        return;
    }
    if (!CharacterMesh->DoesSocketExist(InitialArrowSpawnSocketName) || !CharacterMesh->DoesSocketExist(ArrowHandSocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Bow: Character sockets missing. Spawn='%s' Hand='%s'"),
            *InitialArrowSpawnSocketName.ToString(), *ArrowHandSocketName.ToString());
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerArcherCharacter;
    SpawnParams.Instigator = OwnerArcherCharacter;

    AArcherCharacterBase* OwnerChar = Cast<AArcherCharacterBase>(GetOwner());
    if (!OwnerChar) return;
    
    TSubclassOf<AArrowProjectile> ClassToSpawn = OwnerChar->GetCurrentArrowClass();
    
    // 만약 세팅을 깜빡했다면 기존의 기본 화살을 쓰도록 방어 코드 작성
    if (!ClassToSpawn) ClassToSpawn = ArrowProjectileClass; 

    // 2. 받아온 클래스로 화살 스폰!
    const FTransform InitialSpawnTM = CharacterMesh->GetSocketTransform(InitialArrowSpawnSocketName, RTS_World);
    PreparedArrow = GetWorld()->SpawnActor<AArrowProjectile>(
        ClassToSpawn,
        InitialSpawnTM.GetLocation(),
        InitialSpawnTM.Rotator(),
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
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        ArrowHandSocketName
    );

    GetWorldTimerManager().ClearTimer(NockToStringTimerHandle);
    GetWorldTimerManager().SetTimer(
        NockToStringTimerHandle,
        this,
        &ABow::AttachPreparedArrowToBowString,
        FMath::Max(0.0f, NockToStringDelay),
        false
    );
}

void ABow::DestroyDrawArrow()
{
    UE_LOG(LogTemp, Log, TEXT("Try Destory"));
    GetWorldTimerManager().ClearTimer(NockToStringTimerHandle);

    if (IsValid(PreparedArrow))
    {
        PreparedArrow->Destroy();
        PreparedArrow = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Bow: PreparedArrow Destroyed on [%s]"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
    }
    if (OwnerArcherCharacter && OwnerArcherCharacter->GetMesh())
    {
        TArray<AActor*> AttachedActors;
        OwnerArcherCharacter->GetAttachedActors(AttachedActors); // 캐릭터에 붙은 모든 액터 가져오기

        for (AActor* AttachedActor : AttachedActors)
        {
            // 내가 만든 화살 클래스이면서, 특정 소켓에 붙어있는 놈이라면 얄짤없이 삭제
            if (AttachedActor && AttachedActor->IsA(AArrowProjectile::StaticClass()))
            {
                UE_LOG(LogTemp, Error, TEXT("!!! GHOST ARROW FOUND AND DESTROYED !!!"));
                AttachedActor->Destroy();
            }
        }
    }
}

void ABow::AttachPreparedArrowToBowString()
{
    if (!IsValid(PreparedArrow) || !Mesh)
    {
        return;
    }
    if (!Mesh->DoesSocketExist(BowStringSocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Bow: Bow string socket missing: '%s'"), *BowStringSocketName.ToString());
        return;
    }

    PreparedArrow->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        BowStringSocketName
    );
}

void ABow::ServerEndDraw_Implementation()
{
    if (!bIsCharging) return;

    float ChargePercent = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);

    if (OwnerArcherCharacter)
    {
        OwnerArcherCharacter->ServerPlayFireMontage();
    }
    
    // 먼저 발사 처리 (PreparedArrow는 사용 안 함)
    FireArrow(ChargePercent);
    
    // 2. [핵심] 재장전 쿨타임 시작
    bIsReloading = true;
    UpdateArrowVisual(); // 즉시 화살 제거

    BowState = EBowState::Idle;
    
    // 3. 타이머 설정: ReloadDelayTime 후에 FinishReloading 호출
    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle, 
        this, 
        &ABow::FinishReloading, 
        ReloadDelayTime, 
        false
    );
    
    // 그 다음 상태 변경 (장전 화살 삭제)
    bIsCharging = false;
    ChargeTime = 0.f;
    
    // 서버는 수동으로 호출
    OnRep_IsCharging();
}

void ABow::FinishReloading()
{
    UE_LOG(LogTemp, Error, TEXT("FinishReloading"));
    if (HasAuthority())
    {
        bIsReloading = false;
        if (bIsVisualAiming)
        {
            // 다시 화살을 꺼내는(Nocking) 단계로 강제 진입!
            bIsNocking = true;
            
            // Nocking 타이머를 다시 가동합니다.
            GetWorldTimerManager().SetTimer(
                NockingTimerHandle, 
                this, 
                &ABow::FinishNocking, 
                NockingDelayTime, 
                false
            );
        }
        UpdateArrowVisual();
    }
}

void ABow::FinishNocking()
{
    UE_LOG(LogTemp, Error, TEXT("FinishNocking"));
    bIsNocking = false;
    if (bPendingDraw)
    {
        bPendingDraw = false;
        StartDraw();
    }
}

void ABow::EndDraw()
{
    if (!OwnerArcherCharacter || !OwnerArcherCharacter->IsAiming())
    {
        CancelAction();
        return;
    }
    // PreparedArrow 체크 제거 (더 이상 필요 없음)
    if (bIsCharging)
    {
        // [예측] 서버 응답을 기다리지 않고 내 화면에서 먼저 재장전 상태로 만듭니다.
        bIsReloading = true;
        UpdateArrowVisual();

        ServerEndDraw(); // 서버에 발사 요청
        
        bIsCharging = false;
    }
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
    if (!OwnerArcherCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: OwnerArcherCharacter is NULL"));
        return;
    }
    
    if (FireSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(this, FireSound, GetActorLocation());
    }

    if (!Mesh || !Mesh->DoesSocketExist(BowStringSocketName))
    {
        UE_LOG(LogTemp, Error, TEXT("Bow: Bow string socket invalid: '%s'"), *BowStringSocketName.ToString());
        return;
    }

    // ========== 발사 방향 계산 ==========

    FVector SocketLoc = Mesh->GetSocketLocation(BowStringSocketName);

    FVector ShootDir;
    FVector FinalSpawnLoc = SocketLoc;
    AController* Controller = OwnerArcherCharacter->GetController();
    if (Controller)
    {
        FVector CamLoc;
        FRotator CamRot;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);

        // 카메라 에임 타겟 라인트레이스 (100m)
        FVector AimTarget = CamLoc + CamRot.Vector() * 10000.f;
        FHitResult AimHit;
        FCollisionQueryParams AimParams;
        AimParams.AddIgnoredActor(OwnerArcherCharacter);
        AimParams.AddIgnoredActor(this);
        if (GetWorld()->LineTraceSingleByChannel(AimHit, CamLoc, AimTarget, ECC_Visibility, AimParams))
        {
            AimTarget = AimHit.Location;
        }

        // 카메라 중심선 위에 스폰 (소켓의 forward 거리만큼 앞) → 레티클 정확도 + 머리뒤 아님
        float SocketFwdDist = FVector::DotProduct(SocketLoc - CamLoc, CamRot.Vector());
        FinalSpawnLoc = CamLoc + CamRot.Vector() * FMath::Max(SocketFwdDist, 30.f);
        ShootDir = CamRot.Vector();
    }
    else
    {
        ShootDir = Mesh->GetSocketRotation(BowStringSocketName).Vector();
    }

    // 4. [화살 생성] Instigator 설정 필수
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerArcherCharacter;
    SpawnParams.Instigator = OwnerArcherCharacter; 
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 회전값: ShootDir(조준방향)을 그대로 사용 -> ArrowProjectile에서 Velocity가 0이어도 이 회전값을 씀
    AArcherCharacterBase* OwnerChar = Cast<AArcherCharacterBase>(GetOwner());
    TSubclassOf<AArrowProjectile> ClassToSpawn = ArrowProjectileClass; // 기본값
    
    if (OwnerChar && OwnerChar->GetCurrentArrowClass())
    {
        ClassToSpawn = OwnerChar->GetCurrentArrowClass();
    }

    // 2. 알아낸 클래스로 진짜 화살 스폰!
    AArrowProjectile* FiredArrow = GetWorld()->SpawnActor<AArrowProjectile>(
        ClassToSpawn,
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
        // Delay collision arm by one tick to avoid immediate self-hit/stick at spawn frame.
        FiredArrow->CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        FiredArrow->CollisionBox->IgnoreActorWhenMoving(OwnerArcherCharacter, true);
        FiredArrow->CollisionBox->MoveIgnoreActors.AddUnique(OwnerArcherCharacter);
        FiredArrow->CollisionBox->IgnoreActorWhenMoving(this, true);
        FiredArrow->CollisionBox->MoveIgnoreActors.AddUnique(this);
    }

    // 이펙트 활성화
    if (FiredArrow->TrailNiagara)
    {
        FiredArrow->MulticastActivateTrail();
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

    TWeakObjectPtr<AArrowProjectile> WeakFiredArrow = FiredArrow;
    GetWorldTimerManager().SetTimerForNextTick([WeakFiredArrow]()
    {
        if (!WeakFiredArrow.IsValid() || !WeakFiredArrow->CollisionBox) return;
        WeakFiredArrow->CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    });
    
    // 2. [추가] 캐릭터도 화살을 물리적으로 무시 (이게 핵심입니다!)
    // 캐릭터의 캡슐이 화살을 밀어내지 않게 만듭니다.
    if (FiredArrow->CollisionBox)
    {
        FiredArrow->CollisionBox->IgnoreActorWhenMoving(OwnerArcherCharacter, true);
    }
    OwnerArcherCharacter->MoveIgnoreActorAdd(FiredArrow);
    
    if (OwnerChar && HasAuthority())
    {
        OwnerChar->ConsumeAmmo(OwnerChar->GetCurrentArrowType(), 1);
        
        // (디버그용) 남은 개수 확인
        UE_LOG(LogTemp, Warning, TEXT("발사 완료! 남은 화살 개수: %d"), OwnerChar->GetAmmoCount(OwnerChar->GetCurrentArrowType()));
        
    }
}


void ABow::CancelAction()
{
    UE_LOG(LogTemp, Warning, TEXT("================== [CancelAction] =================="));
    
    GetWorldTimerManager().ClearTimer(NockingTimerHandle);

    bPendingDraw = false;
    bIsNocking = false;
    bIsVisualAiming = false;
    bIsCharging = false;
    BowState = EBowState::Idle;

    if (HasAuthority())
    {
        // 서버는 이 상태를 확정하고 다른 사람들에게 복제합니다.
    }

    UpdateArrowVisual();
}