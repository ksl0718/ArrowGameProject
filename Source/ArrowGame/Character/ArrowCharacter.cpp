#include "ArrowCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h" // 충돌 끄기 위해 필요
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

AArrowCharacter::AArrowCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    //ACharacter는 기본값이 true긴 한데, 명시적으로 적어주면 좋음
    bReplicates = true;
}

// 변수 동기화 등록
void AArrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // bIsDead가 바뀌면 클라이언트에게 알려줌
    DOREPLIFETIME(AArrowCharacter, bIsDead);
}

void AArrowCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    if (DefaultWeaponClass && DefaultWeaponClass->IsChildOf(AWeapon::StaticClass()))
    {   
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = this;

        AWeapon* Spawned = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, Params);
        if (Spawned)
        {
            EquipWeapon(Spawned);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DefaultWeaponClass is invalid or not a Weapon class!"));
    }
}

// 서버에서 실행되는 사망 로직 (GameMode가 호출함)
void AArrowCharacter::Die()
{
    if (bIsDead) return;
    
    // 서버 권한 확인 (혹시 모르니)
    if (HasAuthority())
    {
        bIsDead = true;

        // 서버는 OnRep이 자동 실행 안 되므로 직접 호출해서 사망 처리 수행
        OnRep_IsDead();
    }
}

// 클라이언트 + 서버 모두 실행되는 시각적 사망 처리
void AArrowCharacter::OnRep_IsDead()
{
    if (bIsDead)
    {
        // 사망 애니메이션 재생
        if (DeathMontage)
        {
            PlayMontage(DeathMontage);
        }

        // 충돌 끄기 (시체에 화살이 안 박히게)
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // 입력 막기 (로컬 플레이어인 경우)
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DisableInput(PC);
        }
        
        // 자식 클래스(UserCharacter/AICharacter)별 추가 처리
        HandleDeath();
        OnDeath();
        
        UE_LOG(LogTemp, Warning, TEXT("%s Died (Replicated)"), *GetName());
    }
}

void AArrowCharacter::HandleDeath()
{
    //�ϴ� ���
}

void AArrowCharacter::OnDeath() 
{ 
    //�⺻ ���� ����  - �ڽ��� �������̵� 
}

void AArrowCharacter::EquipWeapon(AWeapon* NewWeapon)
{
    if (NewWeapon == nullptr) return;

    EquippedWeapon = NewWeapon;

    NewWeapon->OnEquip(this);

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        NewWeapon->AttachToComponent(
            MeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("Bow_Socket")   // �� ���� �̸�
        );
    }
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