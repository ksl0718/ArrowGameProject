#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SkillCooldownProvider.generated.h"

class UTexture2D;

UINTERFACE(MinimalAPI)
class USkillCooldownProvider : public UInterface
{
	GENERATED_BODY()
};

class ARROWGAME_API ISkillCooldownProvider
{
	GENERATED_BODY()

public:
	virtual bool GetPrimarySkillHudMeta(UTexture2D*& OutIcon, FText& OutKeyText) const = 0;
	virtual bool GetPrimarySkillCooldown(float& OutRemaining, float& OutDuration) const = 0;
};

