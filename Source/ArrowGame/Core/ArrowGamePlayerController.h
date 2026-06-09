// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArrowGamePlayerController.generated.h"

/**
 * 
 */

class UCountdownWidget;
class UResultWidget;
class UUserWidget;
class UScoreboardWidget;
class URoundTimerWidget;
class USkillCooldownHUDWidget;
class UPauseMenuWidget;
class UHealthBarWidget;
class UHealthComponent;
class UTexture2D;
class USoundBase;
class AArcherCharacterBase;

UCLASS()
class ARROWGAME_API AArrowGamePlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	UFUNCTION(Client, Reliable)
    void Client_SetPlayerEnabledState(bool bPlayerEnabled);
	
	void SetPlayerEnabledState(bool bPlayerEnabled);
	
	// 1. 에디터에서 할당할 입력 에셋들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ScoreboardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* PauseAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UScoreboardWidget> ScoreboardClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UScoreboardWidget* ScoreboardWidget;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	UResultWidget* ResultWidget;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	URoundTimerWidget* RoundTimerWidget;
	
	// 서버가 호출할 클라이언트 전용 함수
	UFUNCTION(Client, Reliable)
	void Client_StartCountdown(float Duration);

	UFUNCTION(Client, Reliable)
	void Client_BattleStart(float TimeLimit);

	UFUNCTION(Client, Reliable)
	void Client_ShowRoundResult(bool bIsWin, float MoveToLobbyInSeconds);

	UFUNCTION(Client, Reliable)
	void Client_ShowHitMarker();

	/** 화살/폭발 충돌음 — 해당 PC 클라이언트에서만 재생 (제3자 미재생) */
	UFUNCTION(Client, Reliable)
	void Client_PlayImpactSound(USoundBase* Sound);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowHitMarker();
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UCountdownWidget> CountdownWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<URoundTimerWidget> RoundTimerWidgetClass;
	
	// ===== Skill Cooldown HUD =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|SkillCooldown")
	TSubclassOf<USkillCooldownHUDWidget> SkillCooldownHUDClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI|SkillCooldown")
	USkillCooldownHUDWidget* SkillCooldownHUDWidget = nullptr;

	// ===== Archer Arrow Icon UI =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Arrow")
	TSubclassOf<UUserWidget> ArrowIconWidgetClass;

	// ===== Pause Menu (ESC) =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Pause")
	TSubclassOf<UPauseMenuWidget> PauseMenuClass;

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void ClosePauseMenu();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetPawn(APawn* InPawn) override;
	
	void ShowScoreboard();
	void HideScoreboard();
	
	// 주기 갱신
	void UpdateSkillCooldownHUD();
	
private:
	void SetPlayerEnabledState_Local(bool bPlayerEnabled);
	void ApplyMovementGateToPawn(APawn* InPawn, bool bAllowMovement);
	void BindLocalHealthBarToPawn();
	
	void ConfigureSkillHUDForCurrentPawn();
	void ConfigureArrowIconForCurrentPawn();

	void TogglePauseMenu();
	void OpenPauseMenu();
	bool CanOpenPauseMenu() const;
	
	bool TryGetCurrentSkillHudMeta(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const;
	bool TryGetCurrentSkillCooldown(int32 SlotIndex, float& OutRemaining, float& OutDuration) const;

	UPROPERTY()
	UHealthBarWidget* HealthBarWidget = nullptr;

	TWeakObjectPtr<UHealthComponent> BoundHealthComp;

	UPROPERTY()
	UUserWidget* LoadingWidget;

	UPROPERTY()
	UCountdownWidget* CountdownWidget;

	UPROPERTY()
	UUserWidget* ArrowIconWidget = nullptr;

	UPROPERTY()
	UPauseMenuWidget* PauseMenuWidget = nullptr;

	bool bPauseMenuOpen = false;
	
	FTimerHandle SkillCooldownUpdateTimerHandle;

	/** RPC가 폰 빙의보다 먼저 올 때 SetPawn에서 입력 잠금을 다시 적용 */
	bool bCachedPlayerEnabled = true;
};
