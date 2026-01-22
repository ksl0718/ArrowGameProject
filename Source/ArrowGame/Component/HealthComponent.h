// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARROWGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;
	// ReplicatedUsing = 함수이름
	// 이거 없으면 변수만 바뀌고 체력바, 피격 모션 등 안나올 수 있음
	// 그래서 OnRep_Health()실행
	// 이 변수 값이 서버에서 바뀌어서 클라이언트에 도착하면 자동으로 OnRep_Health()가 실행
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 0.f;

	// Health 값이 갱신될 때 호출될 함수 (UFUNCTION 필수)
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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
