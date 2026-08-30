#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"
#include "CharacterCustomizeTypes.generated.h"

UENUM(BlueprintType)
enum class ECustomizeSlot : uint8
{
	Hair UMETA(DisplayName = "Hair"),
	Top UMETA(DisplayName = "Top"),
	Vest UMETA(DisplayName = "Vest"),
	Pants UMETA(DisplayName = "Pants"),
	Skirt UMETA(DisplayName = "Skirt"),
	Accessory UMETA(DisplayName = "Accessory")
};

UENUM(BlueprintType)
enum class ECustomizeComponentType : uint8
{
	RootAligned UMETA(DisplayName = "Root Aligned"),
	PrePositioned UMETA(DisplayName = "Pre-Positioned")
};

USTRUCT(BlueprintType)
struct FCharacterCustomizePreset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	TMap<ECustomizeSlot, FPrimaryAssetId> SelectedParts;
	
	bool TryGetSelectedPart(ECustomizeSlot Slot, FPrimaryAssetId& OutPartId) const
	{
		if (const FPrimaryAssetId* FoundPartId = SelectedParts.Find(Slot))
		{
			OutPartId = *FoundPartId;
			return true;
		}	
		return false;
	}
	
	void SetSelectedPart(ECustomizeSlot Slot, const FPrimaryAssetId& PartId)
	{
		SelectedParts.FindOrAdd(Slot) = PartId;
	}
	
	void ClearSelectedPart(ECustomizeSlot Slot)
	{
		SelectedParts.Remove(Slot);
	}
	
	void Reset()
	{
		SelectedParts.Reset();
	}
};


