// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGamePlayerController.h"

void AArrowGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::None;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;

}

void AArrowGamePlayerController::SetPlayerEnabledState(bool bPlayerEnalbed) {

    if (bPlayerEnalbed)
    {
        GetPawn()->EnableInput(this);
    }
    else
    {
        GetPawn()->DisableInput(this);
    }

    bShowMouseCursor = bPlayerEnalbed;
}