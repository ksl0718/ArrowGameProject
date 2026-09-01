#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterCustomizeTypes.h"
#include "CharacterPartCatalog.generated.h"

class UCharacterPartData;

USTRUCT(BlueprintType)
struct FCharacterPartCatalogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	ECustomizeSlot Slot = ECustomizeSlot::Hair;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TArray<TSoftObjectPtr<UCharacterPartData>> Parts;
};

UCLASS(BlueprintType)
class ARROWGAME_API UCharacterPartCatalog : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TArray<FCharacterPartCatalogEntry> Entries;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void GetPartsBySlot(ECustomizeSlot Slot, TArray<UCharacterPartData*>& OutParts) const;
};
