#include "ArrowGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

#ifndef SEARCH_PRESENCE
static const FName SEARCH_PRESENCE(TEXT("SEARCH_PRESENCE"));
#endif

#ifndef SEARCH_LOBBIES
static const FName SEARCH_LOBBIES(TEXT("SEARCH_LOBBIES"));
#endif

#ifndef SEARCH_MINSLOTSAVAILABLE
static const FName SEARCH_MINSLOTSAVAILABLE(TEXT("SEARCH_MINSLOTSAVAILABLE"));
#endif

#ifndef SETTING_MAPNAME
static const FName SETTING_MAPNAME(TEXT("MAPNAME"));
#endif

static const FString LobbyMapPath(TEXT("/Game/ArrowGame/Maps/LobbyMap"));
static const FName MainMenuMapPath(TEXT("/Game/ArrowGame/Maps/MainMenuMap"));

UArrowGameInstance::UArrowGameInstance()
{
    // 기본 초기화
}

void UArrowGameInstance::Init()
{
    Super::Init();

    // 서브시스템 획득 (스팀)
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();

        if (SessionInterface.IsValid())
        {
            // 델리게이트 바인딩
            SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UArrowGameInstance::OnCreateSessionComplete));
            SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UArrowGameInstance::OnFindSessionsComplete));
            SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UArrowGameInstance::OnJoinSessionComplete));
            SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UArrowGameInstance::OnSessionUserInviteAccepted);
        }
    }
}

void UArrowGameInstance::RegisterHostSessionIfNeeded()
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("RegisterHostSessionIfNeeded: SessionInterface invalid"));
        return;
    }

    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("RegisterHostSessionIfNeeded: 세션이 이미 있습니다."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("로비 맵에서 호스트 세션 등록 시작"));
    bCreateSessionAfterDestroy = true;
    bJoinInviteAfterDestroy = false;
    StartCreateSession();
}

void UArrowGameInstance::CreateServer()
{
    UE_LOG(LogTemp, Log, TEXT("세션 생성 시작"));

    if (!SessionInterface.IsValid()) return;

    bCreateSessionAfterDestroy = true;
    bJoinInviteAfterDestroy = false;

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("이전 세션 발견: 파괴 후 생성을 예약합니다."));
        DestroySessionCompleteDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UArrowGameInstance::OnDestroySessionComplete);
        SessionInterface->DestroySession(NAME_GameSession);
    }
    else
    {
        StartCreateSession();
    }
}
void UArrowGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("이전 세션 파괴 완료"));
    
    if (SessionInterface.IsValid())
    {
        // UE5에서는 RemoveDelegate 대신 Remove를 사용합니다.
        SessionInterface->OnDestroySessionCompleteDelegates.Remove(DestroySessionCompleteDelegateHandle);
    }

    if (bJoinInviteAfterDestroy)
    {
        bJoinInviteAfterDestroy = false;
        if (SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("초대 참가를 이어서 진행합니다."));
            SessionInterface->JoinSession(0, NAME_GameSession, PendingInviteResult);
            return;
        }
    }

    if (bCreateSessionAfterDestroy)
    {
        bCreateSessionAfterDestroy = false;
        StartCreateSession();
        return;
    }

    if (bReturnToMainMenuAfterDestroy)
    {
        bReturnToMainMenuAfterDestroy = false;
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::OpenLevel(World, MainMenuMapPath, true);
        }
    }
}

void UArrowGameInstance::StartCreateSession()
{
    UE_LOG(LogTemp, Log, TEXT("StartCreateSession 실행"));

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.NumPublicConnections = 8;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bIsDedicated = false;
    SessionSettings.bUseLobbiesIfAvailable = true; // 스팀 로비 필수
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;

    SessionSettings.BuildUniqueId = 7777;
    SessionSettings.Set(FName("SERVER_NAME_KEY"), FString("SeungRae_Arrow_Game"), static_cast<EOnlineDataAdvertisementType::Type>(2));
    SessionSettings.Set(SETTING_MAPNAME, LobbyMapPath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UArrowGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("세션 생성 실패"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("세션 생성 최종 성공!"));

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FString MapName = World->GetMapName();
    if (MapName.Contains(TEXT("LobbyMap")))
    {
        UE_LOG(LogTemp, Log, TEXT("이미 로비 맵 — ServerTravel 생략 (스팀 초대 URL=LobbyMap)"));
        return;
    }

    World->ServerTravel(TEXT("/Game/ArrowGame/Maps/LobbyMap?listen?game=/Script/ArrowGame.LobbyGameMode"));
}

void UArrowGameInstance::FindServer()
{
    UE_LOG(LogTemp, Log, TEXT("세션 검색 시작"));
    if (!SessionInterface.IsValid()) return;
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 10000;

    // 1. 로비 & 프레즌스 활성화
    SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    // 2. [강력한 필터] 빈자리가 1개 이상 있는 방만 가져와라
    SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

    // 3. [고유 키]
    SessionSearch->QuerySettings.Set(FName("SERVER_NAME_KEY"), FString("SeungRae_Arrow_Game"), EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UArrowGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, 
        FString::Printf(TEXT("검색 성공 여부: %s, 찾은 총 개수: %d"), bWasSuccessful ? TEXT("True") : TEXT("False"), SessionSearch->SearchResults.Num()));

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("스팀에서 총 %d개의 방을 가져왔습니다."), SessionSearch->SearchResults.Num());

        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
        {
            const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
            
            // 🔥 [꼼수 2 핵심] 내가 호스트에 심어둔 비밀번호(7777)가 맞는지 확인!
            if (Result.Session.SessionSettings.BuildUniqueId == 7777)
            {
                FString ServerName;
                Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);

                UE_LOG(LogTemp, Warning, TEXT("찾았다! 방 이름: %s, 핑: %d"), *ServerName, Result.PingInMs);

                // 커스텀 키까지 맞다면 바로 입장
                if (ServerName == "SeungRae_Arrow_Game")
                {
                    SessionInterface->JoinSession(0, NAME_GameSession, Result);
                    return; // 찾았으니 루프 종료
                }
            }
        }
        UE_LOG(LogTemp, Error, TEXT("가져온 리스트 중에 7777번 방이 없습니다..."));
    }
}

void UArrowGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            UE_LOG(LogTemp, Log, TEXT("세션 참가 성공 — ClientTravel: %s"), *ConnectString);
            if (auto* PC = GetFirstLocalPlayerController())
            {
                PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("ClientTravel 실패: PlayerController 없음"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GetResolvedConnectString 실패 (SessionName=%s)"), *SessionName.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("세션 참가 실패: Result=%d"), static_cast<int32>(Result));
    }
}

void UArrowGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invite 수락 실패: SessionInterface invalid"));
        return;
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("스팀 오버레이 참가 요청 확인! 조인을 시작합니다."));

        if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
        {
            UE_LOG(LogTemp, Log, TEXT("기존 세션이 있어 파괴 후 초대 조인을 진행합니다."));
            bCreateSessionAfterDestroy = false;
            bJoinInviteAfterDestroy = true;
            PendingInviteResult = InviteResult;

            DestroySessionCompleteDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UArrowGameInstance::OnDestroySessionComplete);
            SessionInterface->DestroySession(NAME_GameSession);
            return;
        }

        // 찾아온 방 정보(InviteResult)로 바로 조인
        SessionInterface->JoinSession(0, NAME_GameSession, InviteResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("스팀 초대 수락 콜백 실패"));
    }
}
void UArrowGameInstance::SetMatchStartPlayerCount(int32 Count)
{
	MatchStartPlayerCount = Count;
	UE_LOG(LogTemp, Log, TEXT("MatchStartPlayerCount = %d"), MatchStartPlayerCount);
}

void UArrowGameInstance::ReturnToMainMenu()
{
    if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        bReturnToMainMenuAfterDestroy = true;
        bCreateSessionAfterDestroy = false;
        bJoinInviteAfterDestroy = false;

        DestroySessionCompleteDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
            this, &UArrowGameInstance::OnDestroySessionComplete);
        SessionInterface->DestroySession(NAME_GameSession);
        return;
    }

    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, MainMenuMapPath, true);
    }
}
