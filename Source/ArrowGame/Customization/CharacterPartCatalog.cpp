#include "CharacterPartCatalog.h"
#include "CharacterPartData.h"

void UCharacterPartCatalog::GetPartsBySlot(ECustomizeSlot Slot, TArray<UCharacterPartData*>& OutParts) const
{
	OutParts.Reset();
	
	for (const TSoftObjectPtr<UCharacterPartData> PartRef : Parts)
	{
		UCharacterPartData* Part = PartRef.LoadSynchronous();
		if (Part && Part->Slot == Slot)
		{
			OutParts.Add(Part);
		}
	}
}
