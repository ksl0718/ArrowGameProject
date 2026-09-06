#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpiritSightComponent.generated.h"

class AArrowPlayerState;
class USpiritSightMarkerWidget;
class UMaterialInterface;
class UUserWidget;

UENUM(BlueprintType)
enum class ESpiritSightTargetMode : uint8
{
	EnemyTeam,
	DokkaebiOnly,
	ArchersOnly
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARROWGAME_API USpiritSightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiritSightComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Configure(
		UMaterialInterface* InOverlayMaterial,
		TSubclassOf<USpiritSightMarkerWidget> InMarkerWidgetClass,
		TSubclassOf<UUserWidget> InMarkerEntryClass,
		float InScaleNearCm,
		float InScaleFarCm,
		float InScaleAtNear,
		float InScaleAtFar);

	void UpdateSight(bool bActive, ESpiritSightTargetMode TargetMode);
	void ClearSight();

protected:
	UPROPERTY(EditAnywhere, Category = "SpiritSight")
	TObjectPtr<UMaterialInterface> OverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "SpiritSight")
	TSubclassOf<USpiritSightMarkerWidget> MarkerWidgetClass;

	UPROPERTY(EditAnywhere, Category = "SpiritSight")
	TSubclassOf<UUserWidget> MarkerEntryClass;

	UPROPERTY(EditAnywhere, Category = "SpiritSight", meta = (ClampMin = 1.f))
	float ScaleNearCm = 600.f;

	UPROPERTY(EditAnywhere, Category = "SpiritSight", meta = (ClampMin = 1.f))
	float ScaleFarCm = 5000.f;

	UPROPERTY(EditAnywhere, Category = "SpiritSight", meta = (ClampMin = 0.05f, ClampMax = 3.f))
	float ScaleAtNear = 1.05f;

	UPROPERTY(EditAnywhere, Category = "SpiritSight", meta = (ClampMin = 0.05f, ClampMax = 3.f))
	float ScaleAtFar = 0.28f;

	UPROPERTY(EditAnywhere, Category = "SpiritSight")
	FVector MarkerWorldOffset = FVector(0.f, 0.f, 80.f);

	UPROPERTY(EditAnywhere, Category = "SpiritSight")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	void EnsureMarkerWidget();
	bool ShouldShowPlayerState(const AArrowPlayerState* ViewerPS, const AArrowPlayerState* TargetPS, ESpiritSightTargetMode TargetMode) const;
	bool IsOccludedFromView(const APawn* TargetPawn, const FVector& TargetWorldLocation, const FVector& ViewLocation) const;
	void SetOverlayOnSkeletalMeshes(AActor* TargetActor, UMaterialInterface* DesiredOverlay) const;
	void HideMarker();

	UPROPERTY(Transient)
	TObjectPtr<USpiritSightMarkerWidget> MarkerWidget = nullptr;
};
