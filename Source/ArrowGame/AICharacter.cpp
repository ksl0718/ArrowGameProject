// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Bow.h"

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
    bIsDead = true;
    // 공격 타이머 정지
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);

    if (EquippedWeapon)
    {
        EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        if (USkeletalMeshComponent* WeaponMesh = EquippedWeapon->FindComponentByClass<USkeletalMeshComponent>())
        {
            WeaponMesh->SetSimulatePhysics(true);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
            WeaponMesh->SetEnableGravity(true);
        }

        EquippedWeapon->SetOwner(nullptr);   // 더 이상 이 AI의 무기 아님
    }

    // AI의 회전, 공격 등 논리 중단
    CurrentTarget = nullptr;
    SetActorTickEnabled(false); // 더 이상 Tick 돌리지 않음

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]() { Destroy(); }, 3.0f, false);
}

void AAICharacter::TryAttackTarget()
{
    SearchForTarget();

    if (!CurrentTarget) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow) return;

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
    FRotator TargetRot = ToTarget.Rotation();

    SetActorRotation(TargetRot);


    // 바라보는 방향 보정
    SetActorRotation(TargetRot);

    // 2) AI용 단순화된 조준 로직
    Bow->StartAim();

    // 3) 바로 당기기
    Bow->StartDraw();

    // 4) 일정 속도의 발사
    // AI는 ChargeTime 없이 바로 발사해도 됨
    Bow->EndDraw();

    UE_LOG(LogTemp, Log, TEXT("%s fired at %s"),
        *GetName(), *CurrentTarget->GetName());
}