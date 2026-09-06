#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "CharacterCustomizeTypes.generated.h"

UENUM(BlueprintType)
enum class ECustomizeSlot : uint8
{
	Hair UMETA(DisplayName = "Hair"),
	Top UMETA(DisplayName = "Top"),
	Vest UMETA(DisplayName = "Vest"),
	Pants UMETA(DisplayName = "Pants"),
	Socks UMETA(DisplayName = "Socks"),
	Shoes UMETA(DisplayName = "Shoes"),
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
struct FCharacterCustomizePartSelection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	ECustomizeSlot Slot = ECustomizeSlot::Hair;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FSoftObjectPath PartPath;
};

USTRUCT(BlueprintType)
struct FCharacterCustomizePreset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	TArray<FCharacterCustomizePartSelection> SelectedParts;
	
	bool TryGetSelectedPart(ECustomizeSlot Slot, FSoftObjectPath& OutPartPath) const
	{
		for (const FCharacterCustomizePartSelection& SelectedPart : SelectedParts)
		{
			if (SelectedPart.Slot == Slot)
			{
				OutPartPath = SelectedPart.PartPath;
				return true;
			}
		}

		return false;
	}
	
	void SetSelectedPart(ECustomizeSlot Slot, const FSoftObjectPath& PartPath)
	{
		for (FCharacterCustomizePartSelection& SelectedPart : SelectedParts)
		{
			if (SelectedPart.Slot == Slot)
			{
				SelectedPart.PartPath = PartPath;
				return;
			}
		}

		FCharacterCustomizePartSelection NewSelection;
		NewSelection.Slot = Slot;
		NewSelection.PartPath = PartPath;
		SelectedParts.Add(NewSelection);
	}
	
	void ClearSelectedPart(ECustomizeSlot Slot)
	{
		SelectedParts.RemoveAll([Slot](const FCharacterCustomizePartSelection& SelectedPart)
		{
			return SelectedPart.Slot == Slot;
		});
	}
	
	void Reset()
	{
		SelectedParts.Reset();
	}
};


