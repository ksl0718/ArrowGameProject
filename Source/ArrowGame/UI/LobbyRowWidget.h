#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyRowWidget.generated.h"


UCLASS()
class ARROWGAME_API ULobbyRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	
	void Setup(class AArrowPlayerState* InPlayerState);
	
protected:
	// 위젯이 파괴될 때 바인딩을 해제하기 위함
	virtual void NativeDestruct() override;
    
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_PlayerName;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* ReadyStateImg;

	// --- 로직 관련 ---

	// 준비 상태에 따라 도장 가시성을 조절하는 함수
	UFUNCTION()
	void UpdateReadyVisual(bool bIsReady);

private:
	// 메모리 누수 방지를 위해 약포인터 사용
	UPROPERTY()
	TWeakObjectPtr<class AArrowPlayerState> OwningPlayerState;
};
