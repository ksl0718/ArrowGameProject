// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "BurnDamageType.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "../Character/CharacterBase.h"
#include "../Core/GameModes/ArrowGameGameMode.h"
#include "Net/UnrealNetwork.h"

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, Health);
}

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
	}

	// Owner가 있으면 델리게이트 바인딩
	if (GetOwner())
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTaken);
	}

	ArrowGameGameMode = Cast<AArrowGameGameMode>(UGameplayStatics::GetGameMode(this));
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// [핵심] 서버에서 데미지 처리
void UHealthComponent::DamageTaken(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	class  AController* Instigator,
	AActor* DamageCause)
{
	UE_LOG(LogTemp, Warning, TEXT("Component Received Damage! Health: %f"), Health);
	
	if (Damage <= 0.f || bIsDead) return;
	if (!GetOwner()->HasAuthority()) return;
	
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("%s Health: %.1f"), *GetOwner()->GetName(), Health);

	const bool bIsBurnDamage = DamageType && DamageType->IsA(UBurnDamageType::StaticClass());
	const float FlinchStrength = bIsBurnDamage ? BurnHitFlinchStrength : 1.f;

	if (ACharacterBase* CharBase = Cast<ACharacterBase>(GetOwner()))
	{
		if (!bIsBurnDamage)
		{
			FVector HitLocation = GetOwner()->GetActorLocation();
			FVector AttackerLocation = FVector::ZeroVector;
			if (Instigator && Instigator->GetPawn())
				AttackerLocation = Instigator->GetPawn()->GetActorLocation();
			else if (DamageCause)
				AttackerLocation = DamageCause->GetActorLocation();

			CharBase->MulticastHitFX(HitLocation, AttackerLocation);
		}

		CharBase->Multicast_TriggerHitFlinch(FlinchStrength);
	}

	OnRep_Health();

	// 사망 체크
	if (Health <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTimerHandle);
		StopBurnEffects();
		bIsDead = true;
		// 1. 캐릭터에게 "너 죽었어" 알림 (래그돌 실행용)
		OnDead.Broadcast();

		// 2. 게임모드에게 알림 (점수 계산, 리스폰 등)
		if (ArrowGameGameMode)
		{
			ArrowGameGameMode->ActorDied(DamagedActor, Instigator);
		}
	}
}

void UHealthComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UHealthComponent::StartBurn(float Duration, float Interval, float Damage,
	AController* DamageInstigator, UNiagaraSystem* BurnFXOverride)
{
	if (!GetOwner()->HasAuthority() || bIsDead) return;

	BurnTicksRemaining = FMath::Max(1, FMath::FloorToInt(Duration / Interval));
	DamagePerTick = Damage;
	BurnDamageInstigator = DamageInstigator;

	if (!bBurnFXActive)
	{
		if (ACharacterBase* CharBase = Cast<ACharacterBase>(GetOwner()))
		{
			CharBase->MulticastPlayBurnFX(BurnFXOverride);
			bBurnFXActive = true;
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		BurnTimerHandle,
		this,
		&UHealthComponent::ApplyBurnTick,
		Interval,
		true
	);
}

void UHealthComponent::StopBurnEffects()
{
	if (!bBurnFXActive) return;

	if (ACharacterBase* CharBase = Cast<ACharacterBase>(GetOwner()))
	{
		CharBase->MulticastStopBurnFX();
	}
	bBurnFXActive = false;
}

void UHealthComponent::ApplyBurnTick()
{
	if (bIsDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTimerHandle);
		StopBurnEffects();
		return;
	}

	UGameplayStatics::ApplyDamage(
		GetOwner(),
		DamagePerTick,
		BurnDamageInstigator.Get(),
		nullptr,
		UBurnDamageType::StaticClass()
	);

	BurnTicksRemaining--;
	if (BurnTicksRemaining <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTimerHandle);
		StopBurnEffects();
	}
}