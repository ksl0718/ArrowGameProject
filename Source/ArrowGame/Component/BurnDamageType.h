#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "BurnDamageType.generated.h"

/** DoT (burn tick) — use lighter flinch, skip full hit VFX spam. */
UCLASS()
class ARROWGAME_API UBurnDamageType : public UDamageType
{
	GENERATED_BODY()
};
