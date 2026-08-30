#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterCustomizeTypes.h"
#include "CharacterPartData.generated.h"

class USkeletalMesh;
class UTexture2D;

UCLASS(BlueprintType)
class ARROWGAME_API UCharacterPartData : public UPrimaryDataAsset
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	ECustomizeSlot Slot = ECustomizeSlot::Hair;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	ECustomizeComponentType ComponentType = ECustomizeComponentType::RootAligned;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> Mesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization|UI")
	TSoftObjectPtr<UTexture2D> Icon;
};
