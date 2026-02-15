#include "ArrowCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "../Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "../Component/HealthComponent.h"
#include "Components/CapsuleComponent.h"

void AArrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AArrowCharacter, EquippedWeapon);
    DOREPLIFETIME(AArrowCharacter, bIsAiming);
    DOREPLIFETIME(AArrowCharacter, SyncPitch);
}

void AArrowCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (HasAuthority())
    {
        // 서버는 클라이언트가 보낸 정확한 ControlRotation을 이미 알고 있습니다.
        // 이걸 그대로 SyncPitch에 담습니다. (압축 따위 안 함)
        FRotator CurrentRot = GetControlRotation();
        
        // 0~360도를 -180~180도로 깔끔하게 정리해서 저장
        float NormalizedPitch = FRotator::NormalizeAxis(CurrentRot.Pitch);
        
        SyncPitch = NormalizedPitch;
    }
}

AArrowCharacter::AArrowCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bReplicates = true;
    SetReplicateMovement(true);
    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;
    
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false; 
    bUseControllerRotationRoll = false;

    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
    
    GetCharacterMovement()->bOrientRotationToMovement = true; 
    
    //회전 속도 설정 (너무 휙휙 돌면 어색하니까)
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    
    
}

void AArrowCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    if (HasAuthority())
    {
        if (DefaultWeaponClass)
        {
            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.Instigator = this;

            AWeapon* Spawned = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, Params);
            if (Spawned)
            {
                EquipWeapon(Spawned);
            }
        } else
        {
            UE_LOG(LogTemp, Error, TEXT("DefaultWeaponClass is invalid or not a Weapon class!"));
        }
    }
    
    if (HealthComp)
    {
        HealthComp->OnDead.AddDynamic(this, &AArrowCharacter::OnDeathProcessed);
    }
}

void AArrowCharacter::SetRotationMode(bool bAiming)
{
    GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
    if (bAiming)
    {
        // [중요] "나(Local)" 혹은 "서버(Authority)"일 때만 컨트롤러를 따라가야 함.
        // 남의 캐릭터(Proxy)는 컨트롤러가 없으므로 이 옵션을 켜면 고장남!
        if (IsLocallyControlled() || HasAuthority())
        {
            bUseControllerRotationYaw = true;
        }
        else
        {
            // 남의 캐릭터는 서버가 보내주는 회전값을 그대로 받아먹어야 함
            bUseControllerRotationYaw = false; 
        }
    }
    else
    {
        // 조준 풀면 무조건 끔
        bUseControllerRotationYaw = false;
    }
}

void AArrowCharacter::SetAiming(bool bNewAiming)
{
    //FString Role = HasAuthority() ? TEXT("Server") : TEXT("Client");
    //UE_LOG(LogTemp, Warning, TEXT("[%s] 1. SetAiming Called: %d"), *Role, bNewAiming);
    
    bIsAiming = bNewAiming;
    SetRotationMode(bNewAiming);

    if (!HasAuthority())
    {
        ServerSetAiming(bNewAiming);
    }
    else
    {
        ServerSetAiming_Implementation(bNewAiming);
    }
}

void AArrowCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    UE_LOG(LogTemp, Warning, TEXT("[Server] 2. RPC Arrived: %d"), bNewAiming);
    // 변수 동기화 (이미 되어있던 것)
    bIsAiming = bNewAiming;
    
    SetRotationMode(bNewAiming);
    
    // ForceUpdateComponents(); 
}

void AArrowCharacter::OnRep_IsAiming()
{
    UE_LOG(LogTemp, Warning, TEXT("[Client Proxy] 3. OnRep Fired! Value: %d"), bIsAiming);
    // 변수가 서버로부터 도착하면, 자동으로 회전 모드를 바꿈
    SetRotationMode(bIsAiming);
}



void AArrowCharacter::EquipWeapon(AWeapon* NewWeapon)
{
    EquippedWeapon = NewWeapon;

    if (HasAuthority())
    {
        OnRep_EquippedWeapon(); // 서버는 직접 호출
    }
}

void AArrowCharacter::OnRep_EquippedWeapon()
{
    if (!EquippedWeapon) return;

    EquippedWeapon->OnEquip(this);

    EquippedWeapon->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Bow_Socket")
    );
}

// 1. HealthComponent에서 "피 0임!" 하고 신호를 보낼 때 실행되는 함수
void AArrowCharacter::OnDeathProcessed()
{
    // [안전장치] 이미 죽었다면 무시 (중복 실행 방지)
    if (bIsDead) return;
	
    // 서버에서만 사망 로직 시작
    if (HasAuthority())
    {
        // 모든 클라이언트에게 "얘 죽었으니 래그돌 켜라"고 방송
        Multicast_Die();

        // [중요] 서버 전용 로직: 컨트롤러 분리 및 삭제 예약
        // 0.1초 정도 뒤에 컨트롤러를 떼어내어 튕김 현상을 방지합니다.
        GetWorldTimerManager().SetTimerForNextTick([this]()
        {
            if (Controller)
            {
                DetachFromControllerPendingDestroy();
            }
        });

        SetLifeSpan(5.0f); // 5초 뒤 시체 삭제
    }
}

// 2. 모든 클라이언트에서 실행됨
void AArrowCharacter::Multicast_Die_Implementation()
{
    // 중복 실행 방지
    if (bIsDead) return;
    bIsDead = true;

    // 1. 캡슐 콜리전 끄기 (땅 파고 들어가는 현상 방지)
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    // 2. 이동 정지
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    // 3. 래그돌(물리) 실행
    if (GetMesh())
    {
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
        GetMesh()->SetSimulatePhysics(true);
		
        // 죽을 때 화살 쏘는 애니메이션 등을 강제 중단
        UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
        if (AnimInst)
        {
            AnimInst->Montage_Stop(0.2f);
        }
    }
	
    UE_LOG(LogTemp, Warning, TEXT("[%s] Ragdoll Activated"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
}


void AArrowCharacter::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !GetMesh()) return;

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance) return;

    // 1) �̹� ���� ���¸�, Death ����� �ƹ� �͵� ��� ����
    if (bIsDead)
    {
        return;
    }

    AnimInstance->Montage_Play(Montage, PlayRate);
}