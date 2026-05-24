#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstanceBase.generated.h"

/**
 * Shared hit flinch for AnimBP (additive / modify bone via HitFlinchAlpha).
 * Child ABPs read HitFlinchAlpha; no hit montage on the default slot.
 */
UCLASS()
class ARROWGAME_API UCharacterAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Hit")
	void TriggerHitFlinch(float Strength = 1.f);

	/** 0..1, decays in NativeUpdateAnimation — drive additive/modify-bone in ABP. */
	UPROPERTY(BlueprintReadOnly, Category = "Hit")
	float HitFlinchAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	float HitFlinchDecaySpeed = 12.f;

	/** Minimum time between full flinch pulses (DoT-friendly). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	float MinFlinchRetriggerTime = 0.12f;

private:
	float LastFlinchTriggerWorldTime = -1000.f;
};
