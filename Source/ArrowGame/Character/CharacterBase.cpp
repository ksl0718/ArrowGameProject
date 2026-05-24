#include "CharacterBase.h"
#include "CharacterAnimInstanceBase.h"
#include "../Component/HealthComponent.h"
#include "../UI/HealthBarWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Camera/CameraShakeBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// HUD 생성 (기본 제공되는 로컬 플레이어용 체력바)
	if (IsLocallyControlled() && HealthBarClass && HealthComp)
	{
		UHealthBarWidget* HealthWidget = CreateWidget<UHealthBarWidget>(GetWorld(), HealthBarClass);
		if (HealthWidget)
		{
			HealthWidget->AddToViewport();
			HealthComp->OnHealthChanged.AddDynamic(HealthWidget, &UHealthBarWidget::UpdateHealthBar);
			HealthWidget->UpdateHealthBar(HealthComp->GetHealth(), HealthComp->GetMaxHealth());
		}
	}

	if (HealthComp)
	{
		HealthComp->OnDead.AddDynamic(this, &ACharacterBase::OnDeathProcessed);
	}
}

void ACharacterBase::OnDeathProcessed()
{
	if (bIsDead) return;

	// 서버에서만 사망 로직 시작
	if (HasAuthority())
	{
		Multicast_Die();

		// 약간 지연 후에 컨트롤러를 떼어내어 튕김 현상을 완화
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (Controller)
			{
				DetachFromControllerPendingDestroy();
			}
		});

		HandleDeathAdditional();
		SetLifeSpan(5.0f);
	}
}

void ACharacterBase::PlayHitReaction()
{
	ApplyHitFlinch(1.f);
}

void ACharacterBase::ApplyHitFlinch(float Strength)
{
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		if (UCharacterAnimInstanceBase* Anim = Cast<UCharacterAnimInstanceBase>(SkelMesh->GetAnimInstance()))
		{
			Anim->TriggerHitFlinch(Strength);
		}
	}
}

void ACharacterBase::Multicast_TriggerHitFlinch_Implementation(float Strength)
{
	ApplyHitFlinch(Strength);
}

void ACharacterBase::MulticastHitFX_Implementation(FVector HitLocation, FVector AttackerLocation)
{
	if (HitFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitFX, HitLocation,
			FRotator::ZeroRotator, FVector(0.5f));

	if (!IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager && HitCameraShakeClass)
			PC->PlayerCameraManager->StartCameraShake(HitCameraShakeClass);
	}

	OnHitEffect(AttackerLocation);
}

void ACharacterBase::MulticastPlayBurnFX_Implementation(UNiagaraSystem* FXOverride)
{
	MulticastStopBurnFX_Implementation();

	UNiagaraSystem* FX = FXOverride ? FXOverride : BurnFX;
	if (!FX) return;

	USceneComponent* AttachTarget = GetMesh() ? static_cast<USceneComponent*>(GetMesh()) : GetCapsuleComponent();
	if (!AttachTarget) return;

	ActiveBurnFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		FX,
		AttachTarget,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);
}

void ACharacterBase::MulticastStopBurnFX_Implementation()
{
	if (ActiveBurnFX)
	{
		ActiveBurnFX->Deactivate();
		ActiveBurnFX->DestroyComponent();
		ActiveBurnFX = nullptr;
	}
}

void ACharacterBase::Multicast_Die_Implementation()
{
	if (bIsDead) return;
	bIsDead = true;

	MulticastStopBurnFX_Implementation();

	// 1) 캡슐 콜리전 끄기
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 2) 이동 정지
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	// 3) 래그돌
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);

		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.2f);
		}
	}
}

