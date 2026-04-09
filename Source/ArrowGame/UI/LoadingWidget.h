#pragma once

#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

UCLASS()
class ARROWGAME_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* LoadingBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_Status;
	
};
