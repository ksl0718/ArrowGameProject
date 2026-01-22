#include "ArrowCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Weapon.h"

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

    // ????? ??? ??????? / ????? ???? ???
    // e.g. PlayAnimMontage(DeathMontage);
}

void AArrowCharacter::HandleDeath()
{
    //??? ???
}

void AArrowCharacter::OnDeath() 
{ 
    //?? ???? ????  - ????? ????????? 
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
            TEXT("Bow_Socket")   // ?? ???? ???
        );
    }
}

void AArrowCharacter::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !GetMesh()) return;

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance) return;

    // 1) ??? ???? ???¸?, Death ????? ??? ??? ??? ????
    if (bIsDead)
    {
        return;
    }

    AnimInstance->Montage_Play(Montage, PlayRate);
}