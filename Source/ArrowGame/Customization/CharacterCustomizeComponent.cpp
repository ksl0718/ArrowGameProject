#include "CharacterCustomizeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ArrowGame/Customization/CharacterPartData.h"

UCharacterCustomizeComponent::UCharacterCustomizeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterCustomizeComponent::ApplyPart(UCharacterPartData* PartData)
{
	if (!PartData)
	{
		return;
	}
	
	USkeletalMesh* LoadedMesh = PartData->Mesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		return;
	}
	
	ClearSlot(PartData->Slot);
	
	if (USkeletalMeshComponent* TargetComponent = FindComponent(PartData->Slot, PartData->ComponentType))
	{
		TargetComponent->SetSkeletalMesh(LoadedMesh);
		TargetComponent->SetVisibility(true, true);

		CurrentPreset.SetSelectedPart(PartData->Slot, FSoftObjectPath(PartData));
	}
}

void UCharacterCustomizeComponent::ApplyPreset(const FCharacterCustomizePreset& Preset)
{
	UE_LOG(LogTemp, Log, TEXT("ApplyPreset: %d개 파츠 적용 시도"), Preset.SelectedParts.Num());

	for (const FCharacterCustomizePartSelection& SelectedPart : Preset.SelectedParts)
	{
		if (!SelectedPart.PartPath.IsValid())
		{
			continue;
		}

		// 프리셋에는 실제 오브젝트 포인터 대신 에셋 경로만 저장한다.
		// 맵 이동 후 새 캐릭터가 스폰되면 이 경로로 DataAsset을 다시 로드해 적용한다.
		UCharacterPartData* PartData = Cast<UCharacterPartData>(SelectedPart.PartPath.TryLoad());
		if (!PartData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ApplyPreset: 파츠 DataAsset 로드 실패 - %s"), *SelectedPart.PartPath.ToString());
			continue;
		}

		ApplyPart(PartData);
	}
}

FCharacterCustomizePreset UCharacterCustomizeComponent::GetCurrentPreset() const
{
	return CurrentPreset;
}

USkeletalMeshComponent* UCharacterCustomizeComponent::FindComponent(ECustomizeSlot Slot, ECustomizeComponentType ComponentType) const
{
	for (const FCustomizeComponentBinding& Binding : ComponentBindings)
	{
		if (Binding.Slot == Slot && Binding.ComponentType == ComponentType)
		{
			return ResolveComponent(Binding);
		}
	}
	
	return nullptr;
}

USkeletalMeshComponent* UCharacterCustomizeComponent::ResolveComponent(const FCustomizeComponentBinding& Binding) const
{
	if (Binding.ComponentName.IsNone())
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	Owner->GetComponents(SkeletalMeshComponents);

	for (USkeletalMeshComponent* Component : SkeletalMeshComponents)
	{
		if (Component && Component->GetFName() == Binding.ComponentName)
		{
			return Component;
		}
	}

	return nullptr;
}

void UCharacterCustomizeComponent::ClearSlot(ECustomizeSlot Slot)
{
	for (const FCustomizeComponentBinding& Binding : ComponentBindings)
	{
		if (Binding.Slot != Slot)
		{
			continue;
		}

		if (USkeletalMeshComponent* Component = ResolveComponent(Binding))
		{
			Component->SetSkeletalMesh(nullptr);
			Component->SetVisibility(false, true);
		}
	}
}
