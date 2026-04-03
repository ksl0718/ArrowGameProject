#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class ARROWGAME_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	// 플레이어가 서버에 완전히 접속했을 때 호출 (명단 갱신 확인용)
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
