#include "PauseMenuWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "../ArrowGameInstance.h"
#include "../Core/ArrowGamePlayerController.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Okay)
	{
		Btn_Okay->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnOkayClicked);
	}

	if (Btn_Cancle)
	{
		Btn_Cancle->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnCancelClicked);
	}
}

void UPauseMenuWidget::OnOkayClicked()
{
	if (AArrowGamePlayerController* PC = Cast<AArrowGamePlayerController>(GetOwningPlayer()))
	{
		PC->ClosePauseMenu();
	}

	if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(GetGameInstance()))
	{
		ArrowGI->ReturnToMainMenu();
	}
	else if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/ArrowGame/Maps/MainMenuMap")), true);
	}
}

void UPauseMenuWidget::OnCancelClicked()
{
	if (AArrowGamePlayerController* PC = Cast<AArrowGamePlayerController>(GetOwningPlayer()))
	{
		PC->ClosePauseMenu();
	}
	else
	{
		RemoveFromParent();
	}
}
