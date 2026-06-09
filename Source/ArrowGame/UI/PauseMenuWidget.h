#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

UCLASS()
class ARROWGAME_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Okay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Cancle;

	UFUNCTION()
	void OnOkayClicked();

	UFUNCTION()
	void OnCancelClicked();
};
