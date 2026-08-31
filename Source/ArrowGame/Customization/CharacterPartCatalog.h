#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterCustomizeTypes.h"
#include "CharacterPartCatalog.generated.h"

class UCharacterPartData;

UCLASS(BlueprintType)
class ARROWGAME_API UCharacterPartCatalog : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TArray<TSoftObjectPtr<UCharacterPartData>> Parts;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void GetPartsBySlot(ECustomizeSlot Slot, TArray<UCharacterPartData*>& OutParts) const;
	
};
