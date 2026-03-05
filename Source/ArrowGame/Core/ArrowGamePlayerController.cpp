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

void AArrowGamePlayerController::SetPlayerEnabledState(bool bPlayerEnabled) {

    APawn* MyPawn = GetPawn();
    if (MyPawn) // [추가] 폰이 파괴된 상태일 수 있으므로 체크 필수
    {
        if (bPlayerEnabled) MyPawn->EnableInput(this);
        else MyPawn->DisableInput(this);
    }

    bShowMouseCursor = !bPlayerEnabled; // 죽었을 때(false) 마우스를 보여줄지 선택
}