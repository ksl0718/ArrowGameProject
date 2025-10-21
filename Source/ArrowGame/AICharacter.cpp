// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "ArrowProjectile.h"
#include "Components/CapsuleComponent.h"

AAICharacter::AAICharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAICharacter::BeginPlay()
{
    Super::BeginPlay();

    //일정 주기로 타겟 탐색
    GetWorldTimerManager().SetTimer(
        AttackTimerHandle, this, &AAICharacter::TryAttackTarget, AttackInterval, true, 1.f
    );
}

void AAICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //타겟이 있으면 천천히 회전
    if (CurrentTarget)
    {
        FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator TargetRot = ToTarget.Rotation();
        FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, TurnInterpSpeed);
        SetActorRotation(NewRot);
    }
}

void AAICharacter::SearchForTarget()
{
    // 단순히 플레이어 탐색 (1인용 테스트용)
    AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (!Player) return;

    float Distance = FVector::Dist(Player->GetActorLocation(), GetActorLocation());

    if (Distance <= AttackRange)
    {
        CurrentTarget = Player;
    }
    else
    {
        CurrentTarget = nullptr;
    }
}

void AAICharacter::HandleDeath()
{
    Super::HandleDeath();
}

void AAICharacter::OnDeath()
{
    // 공격 타이머 정지
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);

    // AI의 회전, 공격 등 논리 중단
    CurrentTarget = nullptr;
    SetActorTickEnabled(false); // 더 이상 Tick 돌리지 않음

    // 피직스 / 충돌 처리
    /*GetMesh()->SetSimulatePhysics(true);*/
    /*GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]() { Destroy(); }, 3.0f, false);
}

void AAICharacter::TryAttackTarget()
{
    SearchForTarget();

    if (!CurrentTarget) return;

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
    FRotator FireDir = ToTarget.Rotation();

    //FireArrow() 호출
    UE_LOG(LogTemp, Warning, TEXT("%s | SpawnPoint: %s | Class: %s"),
        *GetName(),
        ProjectileSpawnPoint ? TEXT("VALID") : TEXT("NULL"),
        *GetNameSafe(ArrowProjectileClass));
    FireArrow(FireDir, 3000.f);

    UE_LOG(LogTemp, Log, TEXT("%s fired at %s!"), *GetName(), *CurrentTarget->GetName());
}