// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
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

	OnRep_Health();

	// 사망 체크
	if (Health <= 0.f)
	{
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
	// 값이 변해서 이 함수가 불리면, UI에게 알림
	OnHealthChanged.Broadcast(Health, MaxHealth);
	
	if (Health > 0.f)
	{
		if (ACharacterBase* CharBase = Cast<ACharacterBase>(GetOwner()))
		{
			CharBase->PlayHitReaction();
		}
	}
}

void UHealthComponent::StartBurn(float Duration, float Interval, float Damage)
{
	if (!GetOwner()->HasAuthority() || bIsDead) return;

	BurnTicksRemaining = FMath::FloorToInt(Duration / Interval);
	DamagePerTick = Damage;

	// 기존에 불타고 있었다면 타이머 초기화 후 재시작
	GetWorld()->GetTimerManager().SetTimer(
		BurnTimerHandle, 
		this, 
		&UHealthComponent::ApplyBurnTick, 
		Interval, 
		true // 반복 실행
	);
}

void UHealthComponent::ApplyBurnTick()
{
	if (bIsDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTimerHandle);
		return;
	}

	// [중요] 기존 DamageTaken 로직을 재활용합니다! 
	// 그래야 사망 처리, 킬 기록 등이 그대로 연동됩니다.
	// 자기 자신에게 데미지를 입히는 방식
	UGameplayStatics::ApplyDamage(GetOwner(), DamagePerTick, nullptr, nullptr, UDamageType::StaticClass());

	BurnTicksRemaining--;
	if (BurnTicksRemaining <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTimerHandle);
	}
}