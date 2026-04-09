#include "LobbyBoardWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyRowWidget.h"
#include "../Core/ArrowGameState.h"
#include "../Core/ArrowPlayerState.h"


void ULobbyBoardWidget::NativeConstruct()
{
	SetIsFocusable(true);
	
	Super::NativeConstruct();
	
	Super::NativeConstruct();

	// 1. 방장 여부 확인 (보통 서버 권한이 있거나 첫 번째 플레이어인 경우)
	bool bIsHost = GetWorld()->GetNetMode() < NM_Client; // 리슨 서버 호스트 확인

	if (bIsHost)
	{
		// 방장일 때
		Btn_Start->SetVisibility(ESlateVisibility::Visible);
		Btn_Ready->SetVisibility(ESlateVisibility::Collapsed); // 공간까지 아예 없앰
	}
	else
	{
		// 일반 유저일 때
		Btn_Start->SetVisibility(ESlateVisibility::Collapsed);
		Btn_Ready->SetVisibility(ESlateVisibility::Visible);
	}

	// 2. 각각 다른 함수 연결
	if (Btn_Start) Btn_Start->OnClicked.AddDynamic(this, &ULobbyBoardWidget::OnStartClicked);
	if (Btn_Ready) Btn_Ready->OnClicked.AddDynamic(this, &ULobbyBoardWidget::OnReadyClicked);
	
	if (AArrowGameState* GS = GetWorld()->GetGameState<AArrowGameState>())
	{
		// 명단 변경 벨소리에 RefreshPlayerList 연결
		GS->OnPlayerListChanged.AddDynamic(this, &ULobbyBoardWidget::RefreshPlayerList);
	}

	RefreshPlayerList();
}

void ULobbyRowWidget::NativeDestruct()
{
	Super::NativeDestruct();
    
	// 나중에 여기서 델리게이트를 해제(Unbind)할 때 쓸 예정입니다.
	// 지금은 빈 껍데기라도 있어야 '확인할 수 없는 외부 기호' 오류가 사라집니다!
	
}
void ULobbyBoardWidget::RefreshPlayerList()
{
	if (!SB_PlayerList || !RowWidgetClass) return;

	// 1. 기존 리스트 싹 비우기
	SB_PlayerList->ClearChildren();

	// 2. GameState에서 플레이어 목록 가져오기
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GS = World->GetGameState())
		{
			for (APlayerState* PS : GS->PlayerArray)
			{
				if (AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS))
				{
					// 3. Row 위젯 생성 및 데이터 설정
					ULobbyRowWidget* NewRow = CreateWidget<ULobbyRowWidget>(this, RowWidgetClass);
					if (NewRow)
					{
						NewRow->Setup(ArrowPS);
						SB_PlayerList->AddChild(NewRow);
					}
				}
			}
		}
	}
	/**
	if (GetWorld()->GetNetMode() < NM_Client) // 방장 확인
	{
		bool bCanStart = CheckAllPlayersReady(); // 레디 체크 로직을 함수로 분리했다고 가정
		if (btn_Start)
		{
			btn_Start->SetIsEnabled(bCanStart); // 모두 레디 안 했으면 클릭 불가(회색)
		}
	}**/
}

void ULobbyBoardWidget::OnReadyClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Ready Button Clicked!"));
	// 내 PlayerState 찾아서 서버에 레디 신호 보내기
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AArrowPlayerState* PS = PC->GetPlayerState<AArrowPlayerState>())
		{
			// 토글 방식: 누를 때마다 상태 반전
			PS->ServerSetReady(!PS->IsReady());
		}
	}
	
	RefreshPlayerList();
}

void ULobbyBoardWidget::OnStartClicked()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	bool bAllReady = true;

	// 2. 모든 플레이어 상태 확인
	for (APlayerState* PS : GS->PlayerArray)
	{
		AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS);
		if (ArrowPS)
		{
			if (ArrowPS->GetPlayerController() == GetOwningPlayer())
			{
				continue; 
			}

			// 일반 유저가 레디를 안 했다면?
			if (!ArrowPS->IsReady())
			{
				bAllReady = false;
			}
		}
	}

	// 3. 결과 처리
	if (bAllReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 플레이어 준비 완료! 출정 가능!"));
		if (AGameModeBase* CurrentGM = GetWorld()->GetAuthGameMode())
		{
			UE_LOG(LogTemp, Warning, TEXT("심리스 트래블 상태: %s"), CurrentGM->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
		}
		if (UWorld* World = GetWorld())
		{
			// 1. 이동할 맵 경로 (예: /Game/Maps/BattleMap)
			// 2. "?listen" 옵션은 서버로서 대기하겠다는 뜻으로 멀티플레이 이동 시 필수입니다.
			FString MapPath = TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
        
			// 3. ServerTravel은 모든 클라이언트를 동시에 이동시킵니다.
			World->ServerTravel(MapPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("출정 불가"));
	}
}