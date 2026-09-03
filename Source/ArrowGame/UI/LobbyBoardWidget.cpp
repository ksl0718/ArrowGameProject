#include "LobbyBoardWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "LobbyRowWidget.h"
#include "LobbyCustomizationPanelWidget.h"
#include "../ArrowGameInstance.h"
#include "../Core/ArrowGameState.h"
#include "../Core/ArrowPlayerState.h"


void ULobbyBoardWidget::NativeConstruct()
{
	SetIsFocusable(true);
	
	Super::NativeConstruct();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	}

	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

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
	if (Btn_Back) Btn_Back->OnClicked.AddDynamic(this, &ULobbyBoardWidget::OnBackClicked);
	
	if (AArrowGameState* GS = GetWorld()->GetGameState<AArrowGameState>())
	{
		GS->OnPlayerListChanged.AddDynamic(this, &ULobbyBoardWidget::OnLobbyPlayerListChanged);
	}

	RefreshPlayerList();
	ScheduleDelayedPlayerListRefresh();
}

void ULobbyBoardWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayedRefreshTimerHandle);
	}

	if (AArrowGameState* GS = GetWorld() ? GetWorld()->GetGameState<AArrowGameState>() : nullptr)
	{
		GS->OnPlayerListChanged.RemoveDynamic(this, &ULobbyBoardWidget::OnLobbyPlayerListChanged);
	}

	Super::NativeDestruct();
}

void ULobbyBoardWidget::OnLobbyPlayerListChanged()
{
	ScheduleDelayedPlayerListRefresh();
}

void ULobbyBoardWidget::ScheduleDelayedPlayerListRefresh()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(DelayedRefreshTimerHandle);
	World->GetTimerManager().SetTimer(
		DelayedRefreshTimerHandle,
		this,
		&ULobbyBoardWidget::RefreshPlayerList,
		LobbyListRefreshDelaySeconds,
		false);
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
	if (Btn_Start && GetWorld()->GetNetMode() < NM_Client)
	{
		bool bAllReady = true;
		int32 PlayerCount = 0;

		if (AGameStateBase* GS = GetWorld()->GetGameState())
		{
			PlayerCount = GS->PlayerArray.Num();
			for (APlayerState* PS : GS->PlayerArray)
			{
				AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS);
				if (!ArrowPS) continue;
				if (ArrowPS->GetPlayerController() == GetOwningPlayer()) continue;

				if (!ArrowPS->IsReady())
				{
					bAllReady = false;
					break;
				}
			}
		}

		const bool bEnoughPlayers = PlayerCount >= 2;
		Btn_Start->SetIsEnabled(bEnoughPlayers && bAllReady);
	}
}

void ULobbyBoardWidget::OnReadyClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Ready Button Clicked!"));
	CommitLocalCustomizePreset();

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
	CommitLocalCustomizePreset();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	if (GS->PlayerArray.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("출정 불가: 최소 2명 필요"));
		return;
	}

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
			const int32 PlayersLeaving = GS->PlayerArray.Num();
			if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(World->GetGameInstance()))
			{
				ArrowGI->SetMatchStartPlayerCount(PlayersLeaving);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ArrowGameInstance 캐스트 실패 — MatchStartPlayerCount 설정 안 됨"));
			}

			// 전투 GameMode를 URL에 명시 (심리스 2회차 트래블 시 World Settings만으로는 GM이 안 바뀌는 경우 방지)
			const FString TravelURL = TEXT(
				"/Game/HwaseongHaenggung/Maps/HwaseongHaenggung2_2024"
				"?game=/Game/ArrowGame/Blueprint/GameMode/BP_ArrowGameGameMode.BP_ArrowGameGameMode_C");
			World->ServerTravel(TravelURL);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("출정 불가"));
	}
}

void ULobbyBoardWidget::OnBackClicked()
{
	if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(GetGameInstance()))
	{
		ArrowGI->ReturnToMainMenu();
	}
	else if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/ArrowGame/Maps/MainMenuMap")), true);
	}

	RemoveFromParent();
}

bool ULobbyBoardWidget::CommitLocalCustomizePreset()
{
	if (!CustomizationPanel)
	{
		// 커마 패널이 없는 로비 화면도 있을 수 있으므로 실패가 곧 오류는 아니다.
		UE_LOG(LogTemp, Warning, TEXT("LobbyBoard: CustomizationPanel 바인딩 없음 - 커마 프리셋 저장 생략"));
		return false;
	}

	// 확정 타이밍은 로비 보드가 가진다.
	// 클라는 Ready를 누를 때, 방장은 Start를 누를 때 현재 프리뷰 선택값을 PlayerState에 저장한다.
	const bool bCommitted = CustomizationPanel->CommitCurrentPresetToPlayerState();
	UE_LOG(LogTemp, Log, TEXT("LobbyBoard: 커마 프리셋 저장 요청 결과 - %s"), bCommitted ? TEXT("성공") : TEXT("실패"));
	return bCommitted;
}
