#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "ArrowGameInstance.generated.h"

UCLASS()
class UArrowGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 생성자
	UArrowGameInstance();

	// 게임 인스턴스 초기화
	virtual void Init() override;

	// 호스트 세션 생성 (UI 바인딩용, 레거시 — 메인 메뉴에서는 OpenLevel 후 RegisterHostSessionIfNeeded 사용)
	UFUNCTION(BlueprintCallable, Category = "Network")
	void CreateServer();

	// 로비 맵(listen)에 이미 있을 때 온라인 세션만 등록 (스팀 초대 시 맵 URL 일치)
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RegisterHostSessionIfNeeded();

	// 클라이언트 세션 검색 (UI 바인딩용)
	UFUNCTION(BlueprintCallable, Category = "Network")
	void FindServer();


	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetMatchStartPlayerCount(int32 Count);

	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetMatchStartPlayerCount() const { return MatchStartPlayerCount; }

protected:
	// 세션 인터페이스 포인터
	IOnlineSessionPtr SessionInterface;
	
	// 세션 검색 객체
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	// --- 델리게이트 콜백 함수들 ---
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	
	// [추가] 파괴 완료 후 생성을 시작하기 위한 콜백
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// 실제 생성을 담당하는 내부 함수
	void StartCreateSession();
	
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	int32 MatchStartPlayerCount = 0;

	// DestroySession 완료 후 수행할 후속 작업 상태
	bool bCreateSessionAfterDestroy = false;
	bool bJoinInviteAfterDestroy = false;
	FOnlineSessionSearchResult PendingInviteResult;
	
private:
	// 델리게이트 핸들 보관용 (필요 시 해제용)
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
};