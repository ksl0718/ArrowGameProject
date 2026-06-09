// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowGamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../UI/ScoreboardWidget.h"
#include "../UI/CountdownWidget.h"
#include "../UI/ResultWidget.h"
#include "../UI/RoundTimerWidget.h"
#include "../UI/SkillCooldownHUDWidget.h"
#include "../UI/HealthBarWidget.h"
#include "../Character/ArcherCharacterBase.h"
#include "../Character/CharacterBase.h"
#include "../Character/UserArcherCharacter.h"
#include "../Character/DokkaebiCharacter.h"
#include "../Character/SkillCooldownProvider.h"
#include "../Component/HealthComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

void AArrowGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalPlayerController())
    {
        if (ScoreboardClass)
        {
            ScoreboardWidget = CreateWidget<UScoreboardWidget>(this, ScoreboardClass);
        }
        
        if (LoadingWidgetClass)
        {
            LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
            if (LoadingWidget) LoadingWidget->AddToViewport(100);
        }
        
        if (SkillCooldownHUDClass)
        {
            SkillCooldownHUDWidget = CreateWidget<USkillCooldownHUDWidget>(this, SkillCooldownHUDClass);
            if (SkillCooldownHUDWidget)
            {
                SkillCooldownHUDWidget->AddToViewport(5);
                ConfigureSkillHUDForCurrentPawn();
                GetWorldTimerManager().SetTimer(
                    SkillCooldownUpdateTimerHandle,
                    this,
                    &AArrowGamePlayerController::UpdateSkillCooldownHUD,
                    0.05f,
                    true
                );
            }
        }

        ConfigureArrowIconForCurrentPawn();
    }
    
    const FString CurrentMapName = GetWorld() ? GetWorld()->GetMapName() : FString();
    const bool bIsMenuOrLobbyMap = CurrentMapName.Contains(TEXT("MainMenuMap"))
        || CurrentMapName.Contains(TEXT("LobbyMap"));

    if (bIsMenuOrLobbyMap)
    {
        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;

        FInputModeGameAndUI InputMode;
        SetInputMode(InputMode);
    }
    else
    {
        bShowMouseCursor = false;
        DefaultMouseCursor = EMouseCursor::None;

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);

        bEnableClickEvents = false;
        bEnableMouseOverEvents = false;
    }
}

void AArrowGamePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(SkillCooldownUpdateTimerHandle);

    if (HealthBarWidget)
    {
        HealthBarWidget->RemoveFromParent();
        HealthBarWidget = nullptr;
    }
    BoundHealthComp = nullptr;

    if (ArrowIconWidget)
    {
        ArrowIconWidget->RemoveFromParent();
        ArrowIconWidget = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AArrowGamePlayerController::SetPawn(APawn* InPawn)
{
    if (HealthBarWidget && BoundHealthComp.IsValid())
    {
        BoundHealthComp->OnHealthChanged.RemoveDynamic(HealthBarWidget, &UHealthBarWidget::UpdateHealthBar);
        BoundHealthComp = nullptr;
    }

    Super::SetPawn(InPawn);

    if (IsLocalController())
    {
        ConfigureArrowIconForCurrentPawn();
        BindLocalHealthBarToPawn();
        ConfigureSkillHUDForCurrentPawn();

        if (InPawn)
        {
            SetPlayerEnabledState_Local(bCachedPlayerEnabled);
        }
    }
}

void AArrowGamePlayerController::BindLocalHealthBarToPawn()
{
    if (!IsLocalController())
    {
        return;
    }

    ACharacterBase* CharacterPawn = Cast<ACharacterBase>(GetPawn());
    if (!CharacterPawn || !CharacterPawn->HealthComp || !CharacterPawn->HealthBarClass)
    {
        if (HealthBarWidget)
        {
            HealthBarWidget->RemoveFromParent();
        }
        return;
    }

    if (!HealthBarWidget || HealthBarWidget->GetClass() != CharacterPawn->HealthBarClass)
    {
        if (HealthBarWidget)
        {
            HealthBarWidget->RemoveFromParent();
            HealthBarWidget = nullptr;
        }

        HealthBarWidget = CreateWidget<UHealthBarWidget>(this, CharacterPawn->HealthBarClass);
    }

    if (!HealthBarWidget)
    {
        return;
    }

    if (!HealthBarWidget->IsInViewport())
    {
        HealthBarWidget->AddToViewport();
    }

    BoundHealthComp = CharacterPawn->HealthComp;
    BoundHealthComp->OnHealthChanged.AddDynamic(HealthBarWidget, &UHealthBarWidget::UpdateHealthBar);
    HealthBarWidget->UpdateHealthBar(BoundHealthComp->GetHealth(), BoundHealthComp->GetMaxHealth());
}

void AArrowGamePlayerController::ApplyMovementGateToPawn(APawn* InPawn, bool bAllowMovement)
{
    if (AUserArcherCharacter* Archer = Cast<AUserArcherCharacter>(InPawn))
    {
        Archer->bCanMove = bAllowMovement;
    }
    else if (ADokkaebiCharacter* Dokkaebi = Cast<ADokkaebiCharacter>(InPawn))
    {
        Dokkaebi->SetCanMove(bAllowMovement);
    }
}

void AArrowGamePlayerController::UpdateSkillCooldownHUD()
{
    if (!SkillCooldownHUDWidget) return;
    
    const ISkillCooldownProvider* Provider = Cast<ISkillCooldownProvider>(GetPawn());
    if (!Provider) return;
    
    const int32 SlotCount = FMath::Max(0, Provider->GetSkillSlotCount());
    
    for (int32 i = 0; i < SlotCount; ++i)
    {
        float Remaining = 0.f;
        float Duration = 0.01f;
        
        if (Provider->GetSkillCooldownByIndex(i, Remaining, Duration))
        {
            SkillCooldownHUDWidget->UpdateSlotCooldownByIndex(i, Remaining, Duration);
        }
    }
}

void AArrowGamePlayerController::ConfigureSkillHUDForCurrentPawn()
{
    if (!SkillCooldownHUDWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("SkillHUD: SkillCooldownHUDWidget 없음 — PlayerController BP의 Skill Cooldown HUD Class 확인"));
        return;
    }
    SkillCooldownHUDWidget->RefreshFromPawn(GetPawn());
}

void AArrowGamePlayerController::ConfigureArrowIconForCurrentPawn()
{
    if (!IsLocalController()) return;

    const bool bIsArrowCharacter = Cast<AArcherCharacterBase>(GetPawn()) != nullptr;
    if (!bIsArrowCharacter)
    {
        if (ArrowIconWidget)
        {
            ArrowIconWidget->RemoveFromParent();
            ArrowIconWidget = nullptr;
        }
        return;
    }

    if (!ArrowIconWidget && ArrowIconWidgetClass)
    {
        ArrowIconWidget = CreateWidget<UUserWidget>(this, ArrowIconWidgetClass);
    }

    if (ArrowIconWidget && !ArrowIconWidget->IsInViewport())
    {
        ArrowIconWidget->AddToViewport(6);
    }
}

bool AArrowGamePlayerController::TryGetCurrentSkillHudMeta(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const
{
    OutIcon = nullptr;
    OutKeyText = FText::GetEmpty();

    if (const ISkillCooldownProvider* Provider = Cast<ISkillCooldownProvider>(GetPawn()))
    {
        return Provider->GetSkillHudMetaByIndex(SlotIndex, OutIcon, OutKeyText);
    }
    return false;
}

bool AArrowGamePlayerController::TryGetCurrentSkillCooldown(int32 SlotIndex, float& OutRemaining, float& OutDuration) const
{
    OutRemaining = 0.0f;
    OutDuration = 0.01f;

    if (const ISkillCooldownProvider* Provider = Cast<ISkillCooldownProvider>(GetPawn()))
    {
        return Provider->GetSkillCooldownByIndex(SlotIndex, OutRemaining, OutDuration);
    }
    return false;
}

void AArrowGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &AArrowGamePlayerController::ShowScoreboard);
        EnhancedInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Completed, this, &AArrowGamePlayerController::HideScoreboard);
    }
}

void AArrowGamePlayerController::Client_SetPlayerEnabledState_Implementation(bool bPlayerEnabled)
{
    SetPlayerEnabledState_Local(bPlayerEnabled);
}

void AArrowGamePlayerController::SetPlayerEnabledState(bool bPlayerEnabled)
{
    if (HasAuthority())
    {
        ApplyMovementGateToPawn(GetPawn(), bPlayerEnabled);
        Client_SetPlayerEnabledState(bPlayerEnabled);
        if (IsLocalController())
        {
            SetPlayerEnabledState_Local(bPlayerEnabled);
        }
        return;
    }
    
    SetPlayerEnabledState_Local(bPlayerEnabled);
}

void AArrowGamePlayerController::SetPlayerEnabledState_Local(bool bPlayerEnabled)
{
    bCachedPlayerEnabled = bPlayerEnabled;
    ApplyMovementGateToPawn(GetPawn(), bPlayerEnabled);

    if (bPlayerEnabled)
    {
        bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }
    else
    {
        bShowMouseCursor = true;
    }
}

void AArrowGamePlayerController::ShowScoreboard()
{
    if (ScoreboardWidget)
    {
        ScoreboardWidget->RefreshScoreboard();
        ScoreboardWidget->AddToViewport();
    }
}

void AArrowGamePlayerController::HideScoreboard()
{
    if (ScoreboardWidget)
    {
        ScoreboardWidget->RemoveFromParent();
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

void AArrowGamePlayerController::Client_BattleStart_Implementation(float TimeLimit)
{
    SetPlayerEnabledState_Local(true);

    if (CountdownWidget)
    {
        CountdownWidget->RemoveFromParent();
    }
    
    if (RoundTimerWidgetClass)
    {
        RoundTimerWidget = CreateWidget<URoundTimerWidget>(this, RoundTimerWidgetClass);
        if (RoundTimerWidget)
        {
            RoundTimerWidget->AddToViewport();
            RoundTimerWidget->StartTimer(TimeLimit);
        }
    }
}

void AArrowGamePlayerController::Client_ShowRoundResult_Implementation( bool bIsWin, float MoveToLobbyInSeconds)
{
    if (ResultWidget)
    {
        ResultWidget->AddToViewport();
        ResultWidget->showResult(bIsWin);
        FTimerHandle DestroyHandle;
        GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]() {
            if (ResultWidget) ResultWidget->RemoveFromParent();
        }, MoveToLobbyInSeconds, false);
    }
}

void AArrowGamePlayerController::Client_ShowHitMarker_Implementation()
{
	ShowHitMarker();
}

void AArrowGamePlayerController::Client_PlayImpactSound_Implementation(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
