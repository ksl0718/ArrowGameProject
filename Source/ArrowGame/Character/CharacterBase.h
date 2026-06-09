#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"


class UHealthComponent;
class UUserWidget;
class UNiagaraSystem;
class USoundBase;

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

	/** 맞은 본인에게만 재생되는 신음/피격 보이스. 화살 충돌음(ArrowProjectile::HitSound)과 별개. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit FX")
	TArray<TObjectPtr<USoundBase>> HitSounds;

	UFUNCTION(Client, Reliable)
	void Client_PlayLocalHitSound();

	void PlayLocalHitSound();

	/** 사망 시 본인에게만 재생. 캐릭터 BP마다 다른 큐 배열 지정 가능. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death FX")
	TArray<TObjectPtr<USoundBase>> DeathSounds;

	UFUNCTION(Client, Reliable)
	void Client_PlayLocalDeathSound();

	void PlayLocalDeathSound();

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

	/** 로컬 PlayerController HUD가 SetPawn 시 참조 (캐릭터 BeginPlay에서 UI 만들지 않음) */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarClass;
};

