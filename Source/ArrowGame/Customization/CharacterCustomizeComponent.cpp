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

	// 먼저 같은 슬롯에 붙어 있던 컴포넌트들을 모두 비운다.
	// 헤어처럼 한 슬롯 안에서 여러 컴포넌트 후보를 쓰는 경우, 이전 파츠가 남지 않게 하기 위함이다.
	ClearSlot(PartData->Slot);

	if (PartData->bClearSlot)
	{
		// 메시가 없는 선택지도 "현재 선택값"으로 저장해야 Ready/Start 이후에도 그대로 복원된다.
		CurrentPreset.SetSelectedPart(PartData->Slot, FSoftObjectPath(PartData));
		return;
	}

	USkeletalMesh* LoadedMesh = PartData->Mesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyPart: Mesh가 비어 있습니다. None 파츠라면 bClearSlot을 켜세요. - %s"), *PartData->GetName());
		return;
	}

	if (USkeletalMeshComponent* TargetComponent = FindComponent(PartData->Slot, PartData->ComponentType))
	{
		TargetComponent->SetSkeletalMesh(LoadedMesh);
		TargetComponent->SetVisibility(true, true);

		CurrentPreset.SetSelectedPart(PartData->Slot, FSoftObjectPath(PartData));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyPart: 슬롯에 맞는 컴포넌트 바인딩을 찾지 못했습니다. Slot=%d Type=%d"),
			static_cast<int32>(PartData->Slot),
			static_cast<int32>(PartData->ComponentType));
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
