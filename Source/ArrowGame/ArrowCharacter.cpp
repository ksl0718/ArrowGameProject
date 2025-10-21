#include "ArrowCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "ArrowProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

AArrowCharacter::AArrowCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Projectile SpawnPoint"));
    ProjectileSpawnPoint->SetupAttachment(GetMesh());
}

void AArrowCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
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

void AArrowCharacter::FireArrow(const FRotator& FireDir, float Speed)
{
    if (!ArrowProjectileClass || !ProjectileSpawnPoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s tried to fire but missing components."), *GetName());
        return;
    }

    // 스폰 위치
    FVector Location = ProjectileSpawnPoint->GetComponentLocation();

    // 화살 생성
    AArrowProjectile* Projectile = GetWorld()->SpawnActor<AArrowProjectile>(
        ArrowProjectileClass, Location, FireDir
    );

    if (Projectile)
    {
        Projectile->SetOwner(this);

        // 화살의 초기 속도 설정
        Projectile->InitVelocity(Speed);

        UE_LOG(LogTemp, Log, TEXT("%s fired an arrow! Speed: %.1f"), *GetName(), Speed);
    }
}
