#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "CharacterCustomizeTypes.generated.h"

UENUM(BlueprintType)
enum class ECustomizeSlot : uint8
{
	Hair = 0 UMETA(DisplayName = "Hair"),
	Top = 1 UMETA(DisplayName = "Top"),
	Vest = 2 UMETA(DisplayName = "Vest"),
	Pants = 3 UMETA(DisplayName = "Pants"),
	Skirt = 4 UMETA(DisplayName = "Skirt"),
	Accessory = 5 UMETA(DisplayName = "Accessory"),
	Socks = 6 UMETA(DisplayName = "Socks"),
	Shoes = 7 UMETA(DisplayName = "Shoes")
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


