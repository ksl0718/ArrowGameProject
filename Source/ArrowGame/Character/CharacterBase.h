#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"


class UHealthComponent;
class UUserWidget;
class UNiagaraSystem;

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
	/** AnimBP flinch (HitFlinchAlpha). Replaces hit montage. */
	virtual void PlayHitReaction();

	UFUNCTION(BlueprintCallable, Category = "Hit")
	void ApplyHitFlinch(float Strength = 1.f);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TriggerHitFlinch(float Strength);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitFX(FVector HitLocation, FVector AttackerLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hit FX")
	void OnHitEffect(FVector AttackerWorldLocation);

	UPROPERTY(EditAnywhere, Category = "Hit FX")
	UNiagaraSystem* HitFX;

	UPROPERTY(EditAnywhere, Category = "Status FX")
	UNiagaraSystem* BurnFX;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayBurnFX(UNiagaraSystem* FXOverride);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopBurnFX();

	UPROPERTY()
	class UNiagaraComponent* ActiveBurnFX;

	UPROPERTY(EditAnywhere, Category = "Hit FX")
	TSubclassOf<UCameraShakeBase> HitCameraShakeClass;

protected:

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

