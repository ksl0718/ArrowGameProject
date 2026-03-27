// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArrowGamePlayerController.generated.h"

/**
 * 
 */
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
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	void ShowScoreboard();
	void HideScoreboard();
};
