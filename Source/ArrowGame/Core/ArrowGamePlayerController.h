// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArrowGamePlayerController.generated.h"

/**
 * 
 */

class UCountdownWidget;

UCLASS()
class ARROWGAME_API AArrowGamePlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	void SetPlayerEnabledState(bool bPlayerEnalbed);
	
	// 1. 에디터에서 할당할 입력 에셋들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ScoreboardAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UScoreboardWidget> ScoreboardClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UScoreboardWidget* ScoreboardWidget;
	
	
	// 서버가 호출할 클라이언트 전용 함수
	UFUNCTION(Client, Reliable)
	void Client_StartCountdown(float Duration);

	UFUNCTION(Client, Reliable)
	void Client_BattleStart();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UCountdownWidget> CountdownWidgetClass;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	void ShowScoreboard();
	void HideScoreboard();
	
	
private:
	UPROPERTY()
	UUserWidget* LoadingWidget;

	UPROPERTY()
	UCountdownWidget* CountdownWidget;
};
