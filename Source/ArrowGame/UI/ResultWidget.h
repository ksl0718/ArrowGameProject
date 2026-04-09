#pragma once

#include "Blueprint/UserWidget.h"
#include "ResultWidget.Generated.h"

UCLASS()
class ARROWGAME_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_winLose;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_goLobby;
	
	
	UFUNCTION(BlueprintCallable, Category = "Result")
	void showResult(bool bIsWin);
};
