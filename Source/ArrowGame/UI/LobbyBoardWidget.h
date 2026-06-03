#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyBoardWidget.generated.h"

UCLASS()
class ARROWGAME_API ULobbyBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 로비 화면을 새로고침 (플레이어 입장/퇴장 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RefreshPlayerList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** PlayerArray/이름 복제가 끝난 뒤 목록 갱신 (입장 직후 즉시 Refresh는 이름이 비는 경우 있음) */
	void ScheduleDelayedPlayerListRefresh();

	UFUNCTION()
	void OnLobbyPlayerListChanged();

	static constexpr float LobbyListRefreshDelaySeconds = 0.75f;

	FTimerHandle DelayedRefreshTimerHandle;

	// --- UI 바인딩 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> SB_PlayerList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Ready;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Start;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TSubclassOf<class ULobbyRowWidget> RowWidgetClass;

	// --- 버튼 이벤트 ---
	UFUNCTION()
	void OnReadyClicked();
	
	UFUNCTION()
	void OnStartClicked();
};