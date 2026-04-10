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
#include "ArrowGame/Weapon/Bow.h"
#include "Components/CapsuleComponent.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"

void AArrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AArrowCharacter, EquippedWeapon);
    DOREPLIFETIME(AArrowCharacter, bIsAiming);
    DOREPLIFETIME(AArrowCharacter, SyncPitch);
    DOREPLIFETIME(AArrowCharacter, CurrentArrowType);
    DOREPLIFETIME(AArrowCharacter, ArrowAmmoCounts);
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
    
    if (HasAuthority())
    {
        ArrowAmmoCounts.Init(0, static_cast<int32>(EArrowType::Max));
        
        //블루프린트(StartingAmmo)에서 설정한 값들을 배열에 입력
        for (const auto& AmmoPair : StartingAmmo)
        {
            int32 Index = static_cast<int32>(AmmoPair.Key);
            if (ArrowAmmoCounts.IsValidIndex(Index))
            {
                ArrowAmmoCounts[Index] = AmmoPair.Value;
            }
        }
        
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
}

void AArrowCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 폰이 Destroy()될 때(도깨비 배정 후 RestartPlayer 등) 부착 무기 액터가
    // 캐릭터와 함께 반드시 제거되지 않으면 월드에 고아 메시로 남을 수 있음.
    // 이미 Detach된 무기(AI 사망 시 물리 드롭 등)는 건드리지 않음.
    if (HasAuthority() && IsValid(EquippedWeapon))
    {
        if (EquippedWeapon->GetAttachParentActor() == this)
        {
            EquippedWeapon->Destroy();
        }
        EquippedWeapon = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AArrowCharacter::ApplyAimingMovementSettings(bool bAiming)
{
    float TargetSpeed = bAiming ? WalkSpeed : NormalSpeed;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
    }

    // 2. 회전 설정
    GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
    
    if (bAiming)
    {
        // 로컬 플레이어거나 서버일 때만 컨트롤러 회전을 사용
        if (IsLocallyControlled() || (HasAuthority()))
        {
            bUseControllerRotationYaw = true;
        }
    }
    else
    {
        bUseControllerRotationYaw = false;
    }
}

void AArrowCharacter::SetAiming(bool bNewAiming)
{
    // [1. 예측] 서버 응답 기다리지 말고 내 화면(Client) 변수부터 즉시 업데이트
    bIsAiming = bNewAiming; 

    if (HasAuthority())
    {
        // [2. 서버 본인일 때] 직접 활에게 명령
        if (ABow* Bow = Cast<ABow>(EquippedWeapon))
        {
            if (bNewAiming) Bow->StartAim();
            else Bow->StopAim();
        }
    }
    else
    {
        // [3. 클라이언트일 때] 서버에게 "도장 찍어달라고" 요청
        ServerSetAiming(bNewAiming);
    }

    ApplyAimingMovementSettings(bNewAiming);
}

void AArrowCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    SetAiming(bNewAiming);
}

void AArrowCharacter::OnRep_IsAiming()
{
    UE_LOG(LogTemp, Warning, TEXT("[Client Proxy] 3. OnRep Fired! Value: %d"), bIsAiming);
    // 변수가 서버로부터 도착하면, 자동으로 회전 모드를 바꿈
    ApplyAimingMovementSettings(bIsAiming);
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
void AArrowCharacter::ServerPlayCancelMontage_Implementation() { MulticastPlayCancelMontage(); }
void AArrowCharacter::MulticastPlayCancelMontage_Implementation() 
{
    if (CancelMontage)
    {
        PlayAnimMontage(CancelMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing Cancel Montage on All Clients"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CancelMontage is NULL! Check Blueprint."));
    }
}


//------------------화살 관련------------------//

// ----- 탄약 관리 (Getter & Setter) ----- //
int32 AArrowCharacter::GetAmmoCount(EArrowType Type) const
{
    int32 Index = static_cast<int32>(Type);
    if (ArrowAmmoCounts.IsValidIndex(Index))
    {
        return ArrowAmmoCounts[Index];
    }
    return 0;
}

void AArrowCharacter::ConsumeAmmo(EArrowType Type, int32 Amount)
{
    if (!HasAuthority()) return; // 소비는 서버에서만!

    int32 Index = static_cast<int32>(Type);
    if (ArrowAmmoCounts.IsValidIndex(Index))
    {
        ArrowAmmoCounts[Index] = FMath::Max(0, ArrowAmmoCounts[Index] - Amount);
    }
}

void AArrowCharacter::AddAmmo(EArrowType Type, int32 Amount)
{
    if (!HasAuthority()) return; // 획득도 서버에서만!

    int32 Index = static_cast<int32>(Type);
    if (ArrowAmmoCounts.IsValidIndex(Index))
    {
        ArrowAmmoCounts[Index] += Amount;
    }
}
// ----- 화살 교체 로직 ----- //
void AArrowCharacter::EquipArrow(EArrowType NewType)
{
    // 불화살이 1개 이상 있을 때만 교체 가능
    if (GetAmmoCount(NewType) > 0)
    {
        ServerChangeArrowType(NewType);
    }
}

void AArrowCharacter::ServerChangeArrowType_Implementation(EArrowType NewType)
{
    CurrentArrowType = NewType;
    // (선택) 여기서 무기(Bow)에게 "장전된 화살 룩 바꿔!" 라고 명령을 내릴 수도 있습니다.
}

void AArrowCharacter::OnRep_CurrentArrowType()
{
    // 클라이언트 화면에서 화살 종류가 바뀌었을 때 UI 업데이트나 
    // 활에 꽂힌 화살 이펙트(불꽃 등)를 갱신하는 로직을 넣습니다.
}

TSubclassOf<class AArrowProjectile> AArrowCharacter::GetCurrentArrowClass() const
{
    // TMap에 현재 화살 타입이 세팅되어 있다면 그 클래스를 반환
    if (ArrowClasses.Contains(CurrentArrowType))
    {
        return ArrowClasses[CurrentArrowType];
    }
    
    // 세팅을 깜빡했을 때를 대비한 방어 코드 (크래시 방지)
    UE_LOG(LogTemp, Error, TEXT("ArrowClasses TMap에 현재 화살(%d) 클래스가 세팅되지 않았습니다!"), (int32)CurrentArrowType);
    return nullptr; 
}