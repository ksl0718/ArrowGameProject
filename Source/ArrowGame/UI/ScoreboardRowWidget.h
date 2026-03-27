#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardRowWidget.generated.h"

UCLASS()
class ARROWGAME_API UScoreboardRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 블루프린트에서 만든 UpdateRow 기능을 C++에서 정의합니다.
	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void UpdateRow(class AArrowPlayerState* PlayerState);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Kills;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Deaths;
};
