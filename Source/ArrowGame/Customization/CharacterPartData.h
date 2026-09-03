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

	// true면 이 파츠는 새 메시를 장착하지 않고 해당 슬롯을 비우는 선택지로 동작한다.
	// 예: 장신구 없음, 치마 없음처럼 커마에서 일부 파츠를 벗는 항목을 만들 때 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	bool bClearSlot = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> Mesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization|UI")
	TSoftObjectPtr<UTexture2D> Icon;
};
