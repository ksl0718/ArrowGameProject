#pragma once

#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

UCLASS()
class ARROWGAME_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 블루프린트의 ProgressBar와 이름을 맞춰야 합니다.
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UFUNCTION(BlueprintCallable)
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);
};