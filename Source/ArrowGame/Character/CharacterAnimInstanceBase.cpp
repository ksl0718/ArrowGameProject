#include "CharacterAnimInstanceBase.h"

void UCharacterAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (HitFlinchAlpha > KINDA_SMALL_NUMBER)
	{
		HitFlinchAlpha = FMath::FInterpTo(HitFlinchAlpha, 0.f, DeltaSeconds, HitFlinchDecaySpeed);
	}
	else
	{
		HitFlinchAlpha = 0.f;
	}
}

void UCharacterAnimInstanceBase::TriggerHitFlinch(float Strength)
{
	const float ClampedStrength = FMath::Clamp(Strength, 0.f, 1.f);
	if (ClampedStrength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const bool bCanPulse = (Now - LastFlinchTriggerWorldTime) >= MinFlinchRetriggerTime;

	if (bCanPulse)
	{
		HitFlinchAlpha = ClampedStrength;
		LastFlinchTriggerWorldTime = Now;
	}
	else
	{
		HitFlinchAlpha = FMath::Max(HitFlinchAlpha, ClampedStrength * 0.5f);
	}
}
