// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 블루프린트의 버튼 이름이 'Btn_Host'와 정확히 일치해야 합니다.
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Host;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Join;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Quit;
	
	UPROPERTY(meta = (BindWidget))
	class UEditableText* IPAddressInput;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Status;
	
	// 위젯이 생성될 때 실행되는 초기화 함수 (BeginPlay와 비슷함)
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();
	
	UFUNCTION()
	void OnQuitClicked();
	
	bool IsValidIP(const FString& IP);
};
