#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "ArrowGameInstance.generated.h"

class APlayerState;

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

	// 로비 등에서 메인 메뉴로 복귀 (온라인 세션 정리 후 이동)
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetMatchStartPlayerCount(int32 Count);

	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetMatchStartPlayerCount() const { return MatchStartPlayerCount; }

	// 심리스 트래블 중 PlayerState 복사가 실패하거나 새 PlayerState가 만들어지는 경우를 대비한 임시 캐시다.
	// 최종 권위 데이터는 여전히 PlayerState이고, GameInstance는 맵 이동 사이에서만 값을 보관한다.
	void CacheCustomizePresetForPlayer(const APlayerState* PlayerState, const FCharacterCustomizePreset& Preset);

	// 전투맵 도착 후 PlayerState의 커마 값이 비어 있으면 여기서 백업 값을 꺼내 복원한다.
	bool TryGetCachedCustomizePresetForPlayer(const APlayerState* PlayerState, FCharacterCustomizePreset& OutPreset) const;

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

	UPROPERTY()
	TMap<FString, FCharacterCustomizePreset> CachedCustomizePresets;

	// DestroySession 완료 후 수행할 후속 작업 상태
	bool bCreateSessionAfterDestroy = false;
	bool bJoinInviteAfterDestroy = false;
	bool bReturnToMainMenuAfterDestroy = false;
	FOnlineSessionSearchResult PendingInviteResult;

private:
	// 델리게이트 핸들 보관용 (필요 시 해제용)
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FString MakeCustomizePresetCacheKey(const APlayerState* PlayerState) const;
};
