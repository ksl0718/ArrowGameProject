#pragma once

#include "CoreMinimal.h"
#include "UserArcherCharacter.h"
#include "UserCharacter.generated.h"

/**
 * Legacy compatibility wrapper.
 * Keeps old Blueprint parents/casts alive while migrating to AUserArcherCharacter.
 */
UCLASS()
class ARROWGAME_API AUserCharacter : public AUserArcherCharacter
{
	GENERATED_BODY()

public:
	AUserCharacter();
};
