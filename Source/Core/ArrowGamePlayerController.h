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

protected:
	virtual void BeginPlay() override;
	
};
