#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DokkaebiCharacterAnimInstance.generated.h"

/**
 * AnimInstance for ADokkaebiCharacter and ADokkaebiDecoy.
 * Exposes locomotion + skill state for ABP_Dokkaebi (no bow/aim/reload).
 */
UCLASS()
class ARROWGAME_API UDokkaebiCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCanMove(bool bNewCanMove);

	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bIsStealthed = false;

	/** True while curse orb is armed (prepare loop / overlay in ABP). */
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	bool bCurseOrbReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.f;

	/** Alias for blend spaces that still use GroundSpeed. */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float Pitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouched = false;

	UPROPERTY(Transient)
	FVector PrevWorldLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bPrevLocationInitialized = false;
};
