#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"


class UHealthComponent;
class UUserWidget;

UCLASS()
class ARROWGAME_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

	// UHealthComponent의 사망 이벤트에 연결되는 공통 처리
	UFUNCTION()
	void OnDeathProcessed();

	// 사망 VFX/래그돌을 네트워크로 브로드캐스트하는 공통 처리
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Die();

	// 파생 클래스에서 필요하면 추가 사망 연출을 얹을 수 있도록 훅 제공
	virtual void HandleDeathAdditional() {}

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "stats")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarClass;
};

