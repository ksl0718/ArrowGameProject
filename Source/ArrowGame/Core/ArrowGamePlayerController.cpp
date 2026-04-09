// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../UI/ScoreboardWidget.h"
#include "../UI/CountdownWidget.h"
#include "../UI/ResultWidget.h"
#include "GameFramework/PlayerState.h"

void AArrowGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 이 컨트롤러가 로컬(내 컴퓨터)일 때만 위젯 생성
    if (IsLocalPlayerController())
    {
        if (IsLocalPlayerController())
        {
            if (ScoreboardClass)
            {
                // 위젯을 생성하고 포인터 변수에 저장
                ScoreboardWidget = CreateWidget<UScoreboardWidget>(this, ScoreboardClass);
            }
            
            if (LoadingWidgetClass)
            {
                LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
                if (LoadingWidget) LoadingWidget->AddToViewport(100);
            }
            
        }
    }
    
    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::None;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;

}

void AArrowGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // 3. Enhanced Input 컴포넌트로 캐스팅하여 바인딩
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // Started: 키를 눌렀을 때 (Triggered도 가능하지만 탭 키는 보통 Started가 명확합니다)
        EnhancedInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &AArrowGamePlayerController::ShowScoreboard);
		
        // Completed: 키를 뗐을 때
        EnhancedInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Completed, this, &AArrowGamePlayerController::HideScoreboard);
    }
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

void AArrowGamePlayerController::ShowScoreboard()
{
    if (ScoreboardWidget)
    {
        // 3. 점수판을 갱신하고 화면에 띄웁니다.
        ScoreboardWidget->RefreshScoreboard();
        ScoreboardWidget->AddToViewport();
		
        // 마우스 커서가 필요하다면 아래 주석 해제
        // bShowMouseCursor = true;
        // SetInputMode(FInputModeGameAndUI());
    }
}

void AArrowGamePlayerController::HideScoreboard()
{
    if (ScoreboardWidget)
    {
        // 4. 화면에서 지웁니다.
        ScoreboardWidget->RemoveFromParent();
		
        // bShowMouseCursor = false;
        // SetInputMode(FInputModeGameOnly());
    }
}

void AArrowGamePlayerController::Client_StartCountdown_Implementation(float Duration)
{
    if (LoadingWidget) LoadingWidget->RemoveFromParent();

    if (CountdownWidgetClass)
    {
        CountdownWidget = CreateWidget<UCountdownWidget>(this, CountdownWidgetClass);
        if (CountdownWidget)
        {
            CountdownWidget->AddToViewport();
            CountdownWidget->StartCountdown(FMath::RoundToInt(Duration));
        }
    }
}

void AArrowGamePlayerController::Client_BattleStart_Implementation()
{
    if (CountdownWidget)
    {
        // 1초 뒤에 실제로 제거
        FTimerHandle DestroyHandle;
        GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]() {
            if (CountdownWidget) CountdownWidget->RemoveFromParent();
        }, 1.0f, false);
    }
}

void AArrowGamePlayerController::Client_ShowRoundResult_Implementation( bool bIsWin, float MoveToLobbyInSeconds)
{
    if (ResultWidget)
    {
        
        ResultWidget->AddToViewport();
        ResultWidget->showResult(bIsWin);
        // 1초 뒤에 실제로 제거
        FTimerHandle DestroyHandle;
        GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]() {
            if (ResultWidget) ResultWidget->RemoveFromParent();
        }, MoveToLobbyInSeconds, false);
    }
    
    
    /*const FString PlayerName = GetPlayerState<APlayerState>()
        ? GetPlayerState<APlayerState>()->GetPlayerName()
        : TEXT("Unknown");
    
    FTimerHandle ResultHandle;
    GetWorld()->GetTimerManager().SetTimer(ResultHandle, [this, bIsWin, PlayerName]() {
        UE_LOG(LogTemp, Warning, TEXT("GameOver %4s %s"), *PlayerName,  bIsWin ? TEXT("WIN") : TEXT("LOSE"));
    }, MoveToLobbyInSeconds, false); */
}