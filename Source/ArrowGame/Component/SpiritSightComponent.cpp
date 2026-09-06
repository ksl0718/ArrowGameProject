#include "SpiritSightComponent.h"

#include "ArrowGame/Character/CharacterBase.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "ArrowGame/UI/SpiritSightMarkerWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

USpiritSightComponent::USpiritSightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpiritSightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSight();
	if (MarkerWidget)
	{
		MarkerWidget->RemoveFromParent();
		MarkerWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void USpiritSightComponent::Configure(
	UMaterialInterface* InOverlayMaterial,
	TSubclassOf<USpiritSightMarkerWidget> InMarkerWidgetClass,
	TSubclassOf<UUserWidget> InMarkerEntryClass,
	float InScaleNearCm,
	float InScaleFarCm,
	float InScaleAtNear,
	float InScaleAtFar)
{
	OverlayMaterial = InOverlayMaterial;
	MarkerWidgetClass = InMarkerWidgetClass;
	MarkerEntryClass = InMarkerEntryClass;
	ScaleNearCm = InScaleNearCm;
	ScaleFarCm = InScaleFarCm;
	ScaleAtNear = InScaleAtNear;
	ScaleAtFar = InScaleAtFar;

	if (MarkerWidget && MarkerEntryClass)
	{
		MarkerWidget->MarkerEntryWidgetClass = MarkerEntryClass;
	}
}

void USpiritSightComponent::UpdateSight(bool bActive, ESpiritSightTargetMode TargetMode)
{
	APawn* ViewerPawn = Cast<APawn>(GetOwner());
	if (!ViewerPawn || !ViewerPawn->IsLocallyControlled())
	{
		return;
	}

	if (!bActive)
	{
		ClearSight();
		return;
	}

	EnsureMarkerWidget();
	if (!MarkerWidget)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(ViewerPawn->GetController());
	AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	AArrowPlayerState* ViewerPS = ViewerPawn->GetPlayerState<AArrowPlayerState>();
	if (!PC || !GS || !ViewerPS)
	{
		return;
	}

	FVector ViewLocation = ViewerPawn->GetActorLocation();
	if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
	{
		ViewLocation = PCM->GetCameraLocation();
	}

	const float DNear = FMath::Min(ScaleNearCm, ScaleFarCm);
	const float DFar = FMath::Max(ScaleNearCm, ScaleFarCm);

	TArray<FSpiritSightMarkerDrawInfo> MarkerInfos;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AArrowPlayerState* TargetPS = Cast<AArrowPlayerState>(PS);
		if (!ShouldShowPlayerState(ViewerPS, TargetPS, TargetMode))
		{
			continue;
		}

		APawn* TargetPawn = TargetPS->GetPawn();
		if (!TargetPawn || TargetPawn->IsActorBeingDestroyed())
		{
			continue;
		}

		if (ACharacterBase* TargetCharacter = Cast<ACharacterBase>(TargetPawn))
		{
			if (TargetCharacter->bIsDead)
			{
				SetOverlayOnSkeletalMeshes(TargetCharacter, nullptr);
				continue;
			}
		}

		const FVector WorldLocation = TargetPawn->GetActorLocation() + MarkerWorldOffset;
		const bool bOccluded = IsOccludedFromView(TargetPawn, WorldLocation, ViewLocation);
		SetOverlayOnSkeletalMeshes(TargetPawn, bOccluded ? OverlayMaterial.Get() : nullptr);

		if (!bOccluded)
		{
			continue;
		}

		FVector2D WidgetSpace;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, WidgetSpace, true))
		{
			const float DistCm = FVector::Dist(ViewLocation, WorldLocation);
			FSpiritSightMarkerDrawInfo Info;
			Info.ScreenPosition = WidgetSpace;
			Info.UniformScale = FMath::GetMappedRangeValueClamped(
				FVector2D(DNear, DFar),
				FVector2D(ScaleAtNear, ScaleAtFar),
				DistCm);
			MarkerInfos.Add(Info);
		}
	}

	MarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	MarkerWidget->SetSpiritMarkerDrawInfos(MarkerInfos);
}

void USpiritSightComponent::ClearSight()
{
	APawn* ViewerPawn = Cast<APawn>(GetOwner());
	if (!ViewerPawn || !ViewerPawn->IsLocallyControlled())
	{
		return;
	}

	AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	AArrowPlayerState* ViewerPS = ViewerPawn->GetPlayerState<AArrowPlayerState>();
	if (GS && ViewerPS)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			AArrowPlayerState* TargetPS = Cast<AArrowPlayerState>(PS);
			if (!TargetPS || TargetPS == ViewerPS)
			{
				continue;
			}

			if (APawn* TargetPawn = TargetPS->GetPawn())
			{
				SetOverlayOnSkeletalMeshes(TargetPawn, nullptr);
			}
		}
	}

	HideMarker();
}

void USpiritSightComponent::EnsureMarkerWidget()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() || MarkerWidget)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	TSubclassOf<USpiritSightMarkerWidget> WidgetClass = MarkerWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = USpiritSightMarkerWidget::StaticClass();
	}

	MarkerWidget = CreateWidget<USpiritSightMarkerWidget>(PC, WidgetClass);
	if (!MarkerWidget)
	{
		return;
	}

	if (MarkerEntryClass)
	{
		MarkerWidget->MarkerEntryWidgetClass = MarkerEntryClass;
	}

	MarkerWidget->AddToViewport(10);
	MarkerWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
	MarkerWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
	MarkerWidget->SetVisibility(ESlateVisibility::Hidden);
}

bool USpiritSightComponent::ShouldShowPlayerState(
	const AArrowPlayerState* ViewerPS,
	const AArrowPlayerState* TargetPS,
	ESpiritSightTargetMode TargetMode) const
{
	if (!ViewerPS || !TargetPS || TargetPS == ViewerPS)
	{
		return false;
	}

	switch (TargetMode)
	{
	case ESpiritSightTargetMode::EnemyTeam:
		return ViewerPS->IsDokkaebi() != TargetPS->IsDokkaebi();
	case ESpiritSightTargetMode::DokkaebiOnly:
		return TargetPS->IsDokkaebi();
	case ESpiritSightTargetMode::ArchersOnly:
		return !TargetPS->IsDokkaebi();
	default:
		return false;
	}
}

bool USpiritSightComponent::IsOccludedFromView(
	const APawn* TargetPawn,
	const FVector& TargetWorldLocation,
	const FVector& ViewLocation) const
{
	if (!GetWorld() || !TargetPawn)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SpiritSight), false);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(TargetPawn);

	FHitResult Hit;
	return GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TargetWorldLocation, TraceChannel, Params);
}

void USpiritSightComponent::SetOverlayOnSkeletalMeshes(AActor* TargetActor, UMaterialInterface* DesiredOverlay) const
{
	if (!TargetActor)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	TargetActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
	{
		if (IsValid(MeshComp) && MeshComp->GetOverlayMaterial() != DesiredOverlay)
		{
			MeshComp->SetOverlayMaterial(DesiredOverlay);
		}
	}
}

void USpiritSightComponent::HideMarker()
{
	if (!MarkerWidget)
	{
		return;
	}

	MarkerWidget->SetVisibility(ESlateVisibility::Hidden);
	MarkerWidget->SetSpiritMarkerDrawInfos(TArray<FSpiritSightMarkerDrawInfo>());
}
