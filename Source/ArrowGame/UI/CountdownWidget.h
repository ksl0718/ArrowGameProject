#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CountdownWidget.generated.h"

UCLASS()
class ARROWGAME_API UCountdownWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    // 1. 카운트다운 시작 함수
    void StartCountdown(int32 StartValue);

protected:
    // 2. 블루프린트의 위젯/애니메이션과 이름이 같아야 자동으로 연결됨
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* txt_Count;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* CountdownAnimation;

private:
    void UpdateCount();

    FTimerHandle CountdownTimerHandle;
    int32 CurrentCount;
	
};
