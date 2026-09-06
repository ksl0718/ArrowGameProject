#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
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

	
	//커마 파츠 선택값 적용
	// 로비에서 커스터마이징 한 에셋을 실제 캐릭터에 적용할 때 사용
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyPreset(const FCharacterCustomizePreset& Preset);
	
	//컴포넌트에 적용된 파츠 선택 값 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Customization")
	FCharacterCustomizePreset GetCurrentPreset() const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TArray<FCustomizeComponentBinding> ComponentBindings;

private:
	USkeletalMeshComponent* ResolveComponent(const FCustomizeComponentBinding& Binding) const;

	USkeletalMeshComponent* FindComponent(ECustomizeSlot Slot, ECustomizeComponentType ComponentType) const;

	void ClearSlot(ECustomizeSlot Slot);
	
	//현재 액터에 마지막으로 적용된 파츠 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Customization", meta = (AllowPrivateAccess = "true"))
	FCharacterCustomizePreset CurrentPreset;
};
