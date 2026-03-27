#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardWidget.generated.h"

UCLASS()
class ARROWGAME_API UScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void RefreshScoreboard();

protected:
	// meta = (BindWidget)은 BP에 같은 이름의 위젯이 반드시 있어야 함을 의미합니다.
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* VB_PlayerList;

	// 생성할 Row 위젯의 클래스를 에디터에서 지정합니다.
	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TSubclassOf<class UScoreboardRowWidget> RowWidgetClass;
};