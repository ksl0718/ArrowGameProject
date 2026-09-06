#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Customization/CharacterCustomizeTypes.h"
#include "GameFramework/PlayerState.h"
#include "ArrowPlayerState.generated.h"

/**
 * 킬, 데스, 점수 등 플레이어의 게임 데이터를 관리하는 클래스
 */


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyStateChangedDelegate, bool, bNewReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomizePresetChangedDelegate, const FCharacterCustomizePreset&, NewPreset);

UCLASS()
class ARROWGAME_API AArrowPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AArrowPlayerState();

	// 네트워크 복제를 위한 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// SeamlessTravel 중 PlayerState가 새 인스턴스로 복사될 때 커스텀 값도 같이 넘긴다.
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// 재접속/트래블 복원 과정에서 기존 PlayerState 값을 현재 인스턴스에 덮어쓸 때 사용된다.
	virtual void OverrideWith(APlayerState* PlayerState) override;

	// --- Getter ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetDeaths() const { return Deaths; }

	// --- 데이터 수정 (서버에서만 호출) ---
	void AddKill() { Kills++; }
	void AddDeath() { Deaths++; }
	
	bool IsReady() const { return bIsReady; }
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnReadyStateChangedDelegate OnReadyStateChanged;

	UFUNCTION()
	void OnRep_IsReady();
	
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	bool bIsDokkaebi = false;

	/** 서버 전용. 역할 변경 후 즉시 복제되도록 ForceNetUpdate 호출 */
	void SetIsDokkaebi(bool bNewState);
	
	UFUNCTION(BlueprintCallable) // 블루프린트에서도 확인 가능하게
	bool IsDokkaebi() const { return bIsDokkaebi; }

	// 현재 플레이어가 확정한 커마 프리셋을 반환한다.
	// 캐릭터가 스폰된 뒤 이 값을 읽어서 실제 파츠 컴포넌트에 적용한다.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Customization")
	const FCharacterCustomizePreset& GetCustomizePreset() const { return SelectedCustomizePreset; }

	// 로비 UI에서 확정 버튼을 눌렀을 때 호출한다.
	// 클라이언트가 직접 복제 변수 값을 바꾸는 대신 서버에 저장 요청을 보낸다.
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Customization")
	void ServerSetCustomizePreset(const FCharacterCustomizePreset& NewPreset);

	// 서버 코드에서 직접 프리셋을 세팅할 때 사용하는 내부 진입점이다.
	// UI/RPC 경로와 복원 경로가 같은 후처리(알림, 복제 갱신)를 타게 만든다.
	void SetCustomizePresetOnServer(const FCharacterCustomizePreset& NewPreset);

	UPROPERTY(BlueprintAssignable, Category = "Customization")
	FOnCustomizePresetChangedDelegate OnCustomizePresetChanged;

	UFUNCTION()
	void OnRep_SelectedCustomizePreset();
	

protected:
	// Replicated: 서버에서 값이 바뀌면 클라이언트들에게 자동으로 전달됨
	// OnRep_...: 값이 복제되었을 때 UI를 갱신하고 싶다면 사용 (선택사항)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Kills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
	bool bIsReady = false;

	// 플레이어별로 확정된 커마 선택값이다.
	// Pawn이 바뀌거나 SeamlessTravel이 일어나도 PlayerState를 통해 다시 적용할 수 있다.
	UPROPERTY(ReplicatedUsing = OnRep_SelectedCustomizePreset, BlueprintReadOnly, Category = "Customization")
	FCharacterCustomizePreset SelectedCustomizePreset;
};
