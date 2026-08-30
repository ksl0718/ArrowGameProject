#include "CharacterCustomizeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterPartData.h"

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
	}
}

USkeletalMeshComponent* UCharacterCustomizeComponent::FindComponent(ECustomizeSlot Slot, ECustomizeComponentType ComponentType) const
{
	for (const FCustomizeComponentBinding& Binding : ComponentBindings)
	{
		if (Binding.Slot == Slot && Binding.ComponentType == ComponentType)
		{
			return Binding.Component;
		}
	}
	
	return nullptr;
}

void UCharacterCustomizeComponent::ClearSlot(ECustomizeSlot Slot)
{
	for (const FCustomizeComponentBinding& Binding : ComponentBindings)
	{
		if (Binding.Slot == Slot && Binding.Component)
		{
			Binding.Component->SetSkeletalMesh(nullptr);
			Binding.Component->SetVisibility(false, true);
		}
	}
}
