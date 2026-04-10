#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoundTimerWidget.generated.h"

UCLASS()
class ARROWGAME_API URoundTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 컨트롤러가 이 함수를 호출해서 시간을 넘겨줄 겁니다.
	void StartTimer(float Duration);

protected:
	// 블루프린트의 TextBlock 이름과 똑같이 맞춰야 합니다. (예: Txt_Time)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_Time;

private:
	float RemainingTime;
	FTimerHandle UITimerHandle;

	// 1초마다 UI의 글자를 바꿔줄 함수
	void UpdateTimeText();
};