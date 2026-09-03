#include "CharacterCustomizeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
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

		CurrentPreset.SetSelectedPart(PartData->Slot, PartData->GetPrimaryAssetId());
	}
}

void UCharacterCustomizeComponent::ApplyPreset(const FCharacterCustomizePreset& Preset)
{
	for (const TPair<ECustomizeSlot, FPrimaryAssetId>& SelectedPart : Preset.SelectedParts)
	{
		if (!SelectedPart.Value.IsValid())
		{
			continue;
		}

		// 프리셋에는 실제 오브젝트 포인터 대신 PrimaryAssetId만 저장한다.
		// 맵 이동 후에는 AssetManager로 다시 찾아와 적용한다.
		const FSoftObjectPath PartPath = UAssetManager::Get().GetPrimaryAssetPath(SelectedPart.Value);
		UCharacterPartData* PartData = Cast<UCharacterPartData>(PartPath.TryLoad());
		if (!PartData)
		{
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
