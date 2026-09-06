#include "CustomizePartSelectorWidget.h"

#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCustomizePartSelectorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Prev)
	{
		Button_Prev->OnClicked.AddDynamic(this, &UCustomizePartSelectorWidget::SelectPreviousPart);
	}

	if (Button_Next)
	{
		Button_Next->OnClicked.AddDynamic(this, &UCustomizePartSelectorWidget::SelectNextPart);
	}

	UpdateTexts();
}

void UCustomizePartSelectorWidget::NativeDestruct()
{
	if (Button_Prev)
	{
		Button_Prev->OnClicked.RemoveDynamic(this, &UCustomizePartSelectorWidget::SelectPreviousPart);
	}

	if (Button_Next)
	{
		Button_Next->OnClicked.RemoveDynamic(this, &UCustomizePartSelectorWidget::SelectNextPart);
	}

	Super::NativeDestruct();
}

void UCustomizePartSelectorWidget::InitializeSelector(ECustomizeSlot InSlot, const TArray<UCharacterPartData*>& InParts)
{
	CustomizeSlot = InSlot;
	Parts.Reset();

	// 유효한 파츠만 보관해 인덱스 이동 로직에서 null 항목을 따로 처리하지 않게 한다.
	for (UCharacterPartData* Part : InParts)
	{
		if (Part)
		{
			Parts.Add(Part);
		}
	}

	CurrentIndex = 0;

	UpdateTexts();
	ApplyCurrentPart();
}

void UCustomizePartSelectorWidget::SelectPreviousPart()
{
	if (Parts.IsEmpty())
	{
		return;
	}

	// 0번에서 이전 버튼을 눌렀을 때 마지막 인덱스로 자연스럽게 순환시키기 위한 계산이다.
	CurrentIndex = (CurrentIndex - 1 + Parts.Num()) % Parts.Num();

	UpdateTexts();
	ApplyCurrentPart();
}

void UCustomizePartSelectorWidget::SelectNextPart()
{
	if (Parts.IsEmpty())
	{
		return;
	}

	CurrentIndex = (CurrentIndex + 1) % Parts.Num();

	UpdateTexts();
	ApplyCurrentPart();
}

void UCustomizePartSelectorWidget::ApplyCurrentPart()
{
	if (!Parts.IsValidIndex(CurrentIndex))
	{
		return;
	}

	// Selector는 프리뷰 액터나 캐릭터를 직접 알지 않는다.
	// 현재 선택된 파츠 데이터만 외부로 알려준다.
	OnPartChanged.Broadcast(Parts[CurrentIndex]);
}

void UCustomizePartSelectorWidget::UpdateTexts()
{
	if (Text_SlotName)
	{
		Text_SlotName->SetText(GetSlotDisplayText());
	}

	if (Text_Index)
	{
		const FText IndexText = Parts.IsEmpty()
			? FText::FromString(TEXT("0 / 0"))
			: FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentIndex + 1, Parts.Num()));

		Text_Index->SetText(IndexText);
	}
}

FText UCustomizePartSelectorWidget::GetSlotDisplayText() const
{
	switch (CustomizeSlot)
	{
	case ECustomizeSlot::Hair:
		return FText::FromString(TEXT("Hair"));
	case ECustomizeSlot::Top:
		return FText::FromString(TEXT("Top"));
	case ECustomizeSlot::Vest:
		return FText::FromString(TEXT("Vest"));
	case ECustomizeSlot::Pants:
		return FText::FromString(TEXT("Pants"));
	case ECustomizeSlot::Socks:
		return FText::FromString(TEXT("Socks"));
	case ECustomizeSlot::Shoes:
		return FText::FromString(TEXT("Shoes"));
	case ECustomizeSlot::Skirt:
		return FText::FromString(TEXT("Skirt"));
	case ECustomizeSlot::Accessory:
		return FText::FromString(TEXT("Accessory"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}
