#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterCustomizeTypes.h"
#include "CharacterCustomizeComponent.generated.h"

class UCharacterPartData;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FCustomizeComponentBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECustomizeSlot Slot = ECustomizeSlot::Hair;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECustomizeComponentType ComponentType = ECustomizeComponentType::RootAligned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName ComponentName = NAME_None;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARROWGAME_API UCharacterCustomizeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterCustomizeComponent();

	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyPart(UCharacterPartData* PartData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TArray<FCustomizeComponentBinding> ComponentBindings;

private:
	USkeletalMeshComponent* ResolveComponent(const FCustomizeComponentBinding& Binding) const;

	USkeletalMeshComponent* FindComponent(ECustomizeSlot Slot, ECustomizeComponentType ComponentType) const;

	void ClearSlot(ECustomizeSlot Slot);
};
