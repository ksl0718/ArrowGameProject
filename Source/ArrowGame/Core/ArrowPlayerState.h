#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ArrowPlayerState.generated.h"

/**
 * 킬, 데스, 점수 등 플레이어의 게임 데이터를 관리하는 클래스
 */
UCLASS()
class ARROWGAME_API AArrowPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AArrowPlayerState();

	// 네트워크 복제를 위한 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Getter ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetDeaths() const { return Deaths; }

	// --- 데이터 수정 (서버에서만 호출) ---
	void AddKill() { Kills++; }
	void AddDeath() { Deaths++; }

protected:
	// Replicated: 서버에서 값이 바뀌면 클라이언트들에게 자동으로 전달됨
	// OnRep_...: 값이 복제되었을 때 UI를 갱신하고 싶다면 사용 (선택사항)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Kills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths;
};