// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "ArrowGameGameMode.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	// OnTakeAnyDamage 이벤트 등록
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamagerTaken);

	//GameMode 캐스팅 테스트
	ArrowGameGameMode = Cast<AArrowGameGameMode>(UGameplayStatics::GetGameMode(this));
	if (ArrowGameGameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode found: %s"), *ArrowGameGameMode->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode cast failed!"));
	}
	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::DamagerTaken(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	class  AController* Instigator,
	AActor* DamageCause)
{
	if (Damage <= 0.f) return;

	Health -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("%s Health: %.1f"), *GetOwner()->GetName(), Health);

	if (Health <= 0.f && ArrowGameGameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorDied triggered for %s"), *DamagedActor->GetName());
		ArrowGameGameMode->ActorDied(DamagedActor);
	}
}