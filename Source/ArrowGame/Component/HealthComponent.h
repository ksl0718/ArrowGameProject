// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// [추가] 1. 체력이 변했을 때 (UI 갱신용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, CurrentHealth, float, MaxHealth);
// [추가] 2. 사망했을 때 (캐릭터 래그돌용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, Health, float, MaxHealth);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARROWGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	
	// 체력 변경 시 호출될 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
	
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [추가] 외부(UI, 캐릭터)에서 귀를 기울일 방송 채널

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeadDelegate OnDead;
	
	// Sets default values for this component's properties
	UHealthComponent();

	float GetHealth(){ return Health; }
	float GetMaxHealth(){ return MaxHealth; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 0.f;

	UPROPERTY()
	bool bIsDead = false;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0 ? Health / MaxHealth : 0.f; }
	
	UFUNCTION()
	void OnRep_Health();
	
	UFUNCTION()
	void DamageTaken(
		AActor* DamageActor,
		float Damage,
		const UDamageType* DamageType,
		class  AController* Instigator,
		AActor* DamageCause);

	class AArrowGameGameMode* ArrowGameGameMode;
	
};
