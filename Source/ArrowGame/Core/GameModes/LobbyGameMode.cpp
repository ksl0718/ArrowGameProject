#include "LobbyGameMode.h"
#include "ArrowGame/Core/ArrowGameState.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "GameFramework/DefaultPawn.h"

ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
	
	// 1. GameState 클래스 지정 (Base급끼리라 에러 안 남)
	GameStateClass = AArrowGameState::StaticClass();

	// 2. 다른 클래스들도 본인 프로젝트 이름에 맞게 지정
	PlayerControllerClass = APlayerController::StaticClass();
	PlayerStateClass = AArrowPlayerState::StaticClass();

	// 3. 로비에서 사용할 기본 폰 (없으면 기본값)
	DefaultPawnClass = ADefaultPawn::StaticClass();
	
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 새로운 플레이어가 들어오면 로그를 남겨서 확인
	if (NewPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lobby 접속 완료: %s"), *NewPlayer->GetName());
	}
}