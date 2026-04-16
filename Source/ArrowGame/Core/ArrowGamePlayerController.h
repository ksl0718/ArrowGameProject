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
class UTexture2D;

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
	
	// 에디터에서 아이콘 넣기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|SkillCooldown")
	
	UTexture2D* DecoySkillIcon = nullptr;
	
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

	UPROPERTY()
	UUserWidget* LoadingWidget;

	UPROPERTY()
	UCountdownWidget* CountdownWidget;
	
	FTimerHandle SkillCooldownUpdateTimerHandle;
	bool bCachedPlayerEnabled = true;
};
