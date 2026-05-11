#include "SpiritSightMarkerWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

void USpiritSightMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureCanvasRoot();
}

void USpiritSightMarkerWidget::EnsureCanvasRoot()
{
	if (RootMarkerCanvas)
	{
		return;
	}

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!Canvas)
	{
		UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		WidgetTree->RootWidget = Overlay;
		Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		if (UOverlaySlot* OverlaySlot = Overlay->AddChildToOverlay(Canvas))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	RootMarkerCanvas = Canvas;
}

void USpiritSightMarkerWidget::EnsureMarkerPoolSize(int32 NeededVisible)
{
	EnsureCanvasRoot();
	if (!RootMarkerCanvas || !MarkerEntryWidgetClass)
	{
		return;
	}

	NeededVisible = FMath::Clamp(NeededVisible, 0, MaxMarkersCap);

	while (MarkerEntries.Num() < NeededVisible)
	{
		UUserWidget* Entry = CreateWidget<UUserWidget>(this, MarkerEntryWidgetClass);
		if (!Entry)
		{
			break;
		}

		UCanvasPanelSlot* PanelSlot = RootMarkerCanvas->AddChildToCanvas(Entry);
		PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		PanelSlot->SetAlignment(FVector2D(0.f, 0.f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		Entry->SetVisibility(ESlateVisibility::Collapsed);

		MarkerEntries.Add(Entry);
		MarkerSlots.Add(PanelSlot);
	}
}

void USpiritSightMarkerWidget::SetSpiritMarkerDrawInfos_Implementation(const TArray<FSpiritSightMarkerDrawInfo>& DrawInfos)
{
	if (!MarkerEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpiritSightMarkerWidget: MarkerEntryWidgetClass 없음 — 에디터 Class Defaults에서 WBP 지정"));
		return;
	}

	const int32 VisibleCount = FMath::Min(DrawInfos.Num(), MaxMarkersCap);
	EnsureMarkerPoolSize(VisibleCount);

	for (int32 i = 0; i < MarkerEntries.Num(); ++i)
	{
		UUserWidget* Entry = MarkerEntries[i];
		UCanvasPanelSlot* PanelSlot = MarkerSlots.IsValidIndex(i) ? MarkerSlots[i] : nullptr;
		if (!Entry || !PanelSlot)
		{
			continue;
		}

		if (!DrawInfos.IsValidIndex(i) || i >= VisibleCount)
		{
			Entry->SetRenderScale(FVector2D(1.f, 1.f));
			Entry->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const FSpiritSightMarkerDrawInfo& Info = DrawInfos[i];
		FVector2D Pos = Info.ScreenPosition;
		Pos -= MarkerPivotOffset;
		PanelSlot->SetPosition(Pos);

		const float S = FMath::Clamp(Info.UniformScale, 0.05f, 3.f);
		Entry->SetRenderScale(FVector2D(S, S));
		Entry->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
