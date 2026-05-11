#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpiritSightMarkerWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;

USTRUCT(BlueprintType)
struct FSpiritSightMarkerDrawInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiritSight")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	/** 1 = WBP 디자인 크기, 멀수록 작게 넘김 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiritSight")
	float UniformScale = 1.f;
};

/**
 * 도깨비 투시: Canvas 위에 MarkerEntryWidgetClass(WBP) 인스턴스를 적 수만큼만 붙였다 뗐다(풀 재사용).
 * 궁수/적 인원이 바뀌어도 MaxMarkersCap 까지만 늘어남.
 */
UCLASS()
class ARROWGAME_API USpiritSightMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 마커 한 개 분량 WBP (이미지·애니는 여기만 만들면 됨) */
	UPROPERTY(EditDefaultsOnly, Category = "SpiritSight")
	TSubclassOf<UUserWidget> MarkerEntryWidgetClass;

	/** 동시 마커 상한 (플레이어 수 폭주 방지) */
	UPROPERTY(EditDefaultsOnly, Category = "SpiritSight", meta = (ClampMin = 1, ClampMax = 64))
	int32 MaxMarkersCap = 24;

	/**
	 * Canvas 슬롯 위치는 좌상단 기준. 마커 WBP 중심을 월드 투영점에 맞추려면 (너비/2, 높이/2) 정도 넣기.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "SpiritSight")
	FVector2D MarkerPivotOffset = FVector2D(24.f, 32.f);

	UFUNCTION(BlueprintNativeEvent, Category = "SpiritSight")
	void SetSpiritMarkerDrawInfos(const TArray<FSpiritSightMarkerDrawInfo>& DrawInfos);
	virtual void SetSpiritMarkerDrawInfos_Implementation(const TArray<FSpiritSightMarkerDrawInfo>& DrawInfos);

protected:
	virtual void NativeConstruct() override;

private:
	void EnsureCanvasRoot();
	void EnsureMarkerPoolSize(int32 NeededVisible);

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootMarkerCanvas;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> MarkerEntries;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCanvasPanelSlot>> MarkerSlots;
};
