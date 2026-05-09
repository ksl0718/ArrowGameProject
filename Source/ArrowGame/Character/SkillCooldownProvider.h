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
	// 슬롯 개수
	virtual int32 GetSkillSlotCount() const = 0;
	// 슬롯별 아이콘/키
	virtual bool GetSkillHudMetaByIndex(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const = 0;
	// 슬롯별 쿨다운
	virtual bool GetSkillCooldownByIndex(int32 SlotIndex, float& OutRemaining, float& OutDuration) const = 0;
};

