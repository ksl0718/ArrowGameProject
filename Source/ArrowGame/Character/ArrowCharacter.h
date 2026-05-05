#pragma once

#include "CoreMinimal.h"
#include "ArcherCharacterBase.h"
#include "ArrowCharacter.generated.h"

/**
 * Legacy compatibility wrapper.
 * Keeps old Blueprint parents/casts alive while migrating to AArcherCharacterBase.
 */
UCLASS()
class ARROWGAME_API AArrowCharacter : public AArcherCharacterBase
{
	GENERATED_BODY()

public:
	AArrowCharacter();
};
