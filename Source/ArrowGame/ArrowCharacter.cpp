#include "ArrowCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AArrowCharacter::AArrowCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
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

void AArrowCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    HandleDeath();

    OnDeath();

    UE_LOG(LogTemp, Warning, TEXT("%s has died."), *GetName());

    // 나중에 사망 애니메이션 / 이펙트 넣을 자리
    // e.g. PlayAnimMontage(DeathMontage);
}

void AArrowCharacter::HandleDeath()
{
    //일단 비워
}

void AArrowCharacter::OnDeath() 
{ 
    //기본 구현 없음  - 자식이 오버라이드 
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
            TEXT("Bow_socket")   // ← 소켓 이름
        );
    }
}

void AArrowCharacter::PlayFireMontage()
{
	if (FireMontage)
	{
		PlayAnimMontage(FireMontage);
	}
}