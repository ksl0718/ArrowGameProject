// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/ArrowCharacter.h"
#include "../Core/ArrowGameGameMode.h"
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
	PrimaryComponentTick.bCanEverTick = true;
	
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
	
	AArrowCharacter* ArrowChar = Cast<AArrowCharacter>(GetOwner());
	if (ArrowChar)
	{
		// 🔥 [핵심 추가] 체력이 0보다 크고, 몽타주가 있고, "구르는 중이 아닐 때만(!!!)" 재생
		if (Health > 0.f && ArrowChar->HitMontage && !ArrowChar->IsRolling())
		{
			ArrowChar->PlayMontage(ArrowChar->HitMontage);
		}
		else if (ArrowChar->IsRolling())
		{
			// (선택 사항) 구르는 중이라 모션은 안 틀지만, 맞았다는 타격감을 위해 
			// 핏물 튀기는 파티클(VFX)이나 윽! 하는 사운드만 여기서 따로 틀어줘도 아주 좋습니다.
			UE_LOG(LogTemp, Log, TEXT("구르는 중에 맞아서 피격 모션을 생략합니다."));
		}
	}
}
