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


void AArrowCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    HandleDeath();

    OnDeath();

    UE_LOG(LogTemp, Warning, TEXT("%s has died."), *GetName());

    // ���߿� ��� �ִϸ��̼� / ����Ʈ ���� �ڸ�
    // e.g. PlayAnimMontage(DeathMontage);
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

// 1. 컴포넌트나 외부에서 호출하는 사망 처리 함수
void AArrowCharacter::HandleDeath()
{
    // 가상 함수인 OnDeath를 호출해서 자식 클래스(AI 등)가 덮어쓸 수 있게 함
    OnDeath();
}

// 2. 실제 사망 로직 (가상 함수)
void AArrowCharacter::OnDeath()
{
    // 서버에서 래그돌 방송(Multicast)을 실행
    Multicast_Die();

    // 컨트롤러 분리 (플레이어가 더 이상 조작 못하게)
    DetachFromControllerPendingDestroy();
    
    // 5초 뒤 시체 삭제
    SetLifeSpan(5.0f);
}


// 1. 컴포넌트가 죽었다고 신호를 보냄 (서버에서 실행됨)
void AArrowCharacter::OnDeathProcessed()
{
    // 서버가 "모든 클라이언트들아, 얘 래그돌 만들어라!" 명령
    Multicast_Die();

    // (선택사항) 컨트롤러 분리
    DetachFromControllerPendingDestroy();
}

// 2. 모든 클라이언트에서 실행됨
void AArrowCharacter::Multicast_Die_Implementation()
{
    // 1. 캡슐 콜리전 끄기 (시체가 붕 뜨지 않게)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 2. 이동 멈추기
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // 3. 래그돌 실행 (Mesh Physics 켜기)
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);

    // 4. 시체 5초 뒤 삭제
    SetLifeSpan(5.0f);
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