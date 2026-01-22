// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Weapon/Bow.h"

AAICharacter::AAICharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAICharacter::BeginPlay()
{
    Super::BeginPlay();
    if (EquippedWeapon) {
		ABow* Bow = Cast<ABow>(EquippedWeapon);
    }
    //���� �ֱ�� Ÿ�� Ž��
    GetWorldTimerManager().SetTimer(
        AttackTimerHandle, this, &AAICharacter::TryAttackTarget, AttackInterval, true, 1.f
    );
}

void AAICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //Ÿ���� ������ õõ�� ȸ��
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
    // �ܼ��� �÷��̾� Ž�� (1�ο� �׽�Ʈ��)
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
    // ���� Ÿ�̸� ����
 
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

        EquippedWeapon->SetOwner(nullptr);   // �� �̻� �� AI�� ���� �ƴ�
    }

    // AI�� ȸ��, ���� �� �� �ߴ�
    CurrentTarget = nullptr;
    SetActorTickEnabled(false); // �� �̻� Tick ������ ����

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]() { Destroy(); }, 3.0f, false);
}

void AAICharacter::TryAttackTarget()
{
    if (bIsDead) return;
    if (!bCanAttack) return;

    SearchForTarget();

    if (!CurrentTarget) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow) return;

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
    FRotator TargetRot = ToTarget.Rotation();

    SetActorRotation(TargetRot);


    // �ٶ󺸴� ���� ����
    SetActorRotation(TargetRot);
    BeginAIAim();
}

void AAICharacter::BeginAIAim()
{
    if (bIsDead) return;

	ABow* Bow = Cast<ABow>(EquippedWeapon);
	if (Bow)
	{
        bCanAttack = false;
		Bow->StartAim();

        GetWorldTimerManager().SetTimer(
            AIDrawTimerHandle,
            this,
            &AAICharacter::BeginAIDraw,
            1.f,
            false
        );
	}
}

void AAICharacter::BeginAIDraw()
{
    if (bIsDead) return;

	ABow* Bow = Cast<ABow>(EquippedWeapon);
	if (Bow)
	{
		Bow->StartDraw();
        GetWorldTimerManager().SetTimer(
            AIFireTimerHandle,
            this,
            &AAICharacter::EndAIDrawAndFire,
            2.f,
            false
        );
	}
}

void AAICharacter::EndAIDrawAndFire()
{
    if (bIsDead) return;

	ABow* Bow = Cast<ABow>(EquippedWeapon);
	if (Bow)
	{
		Bow->EndDraw();
	}

	//���� ��Ÿ�� ����
	GetWorldTimerManager().SetTimer(
		AttackCooldownHandle,
		this,
		&AAICharacter::ResetAttackCooldown,
		3.f,
		false
	);
}

void AAICharacter::ResetAttackCooldown()
{
	bCanAttack = true;
}