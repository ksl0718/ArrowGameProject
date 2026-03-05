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

void UArrowGameInstance::CreateServer()
{
    UE_LOG(LogTemp, Log, TEXT("세션 생성 시작"));

    if (!SessionInterface.IsValid()) return;

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
    UE_LOG(LogTemp, Log, TEXT("이전 세션 파괴 완료: 새 세션을 생성합니다."));
    
    if (SessionInterface.IsValid())
    {
        // UE5에서는 RemoveDelegate 대신 Remove를 사용합니다.
        SessionInterface->OnDestroySessionCompleteDelegates.Remove(DestroySessionCompleteDelegateHandle);
    }

    StartCreateSession();
}

void UArrowGameInstance::StartCreateSession()
{
    UE_LOG(LogTemp, Log, TEXT("StartCreateSession 실행"));

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.NumPublicConnections = 2;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bIsDedicated = false;
    SessionSettings.bUseLobbiesIfAvailable = true; // 스팀 로비 필수
    SessionSettings.bAllowJoinInProgress = true;

    SessionSettings.BuildUniqueId = 7777;
    SessionSettings.Set(FName("SERVER_NAME_KEY"), FString("SeungRae_Arrow_Game"), static_cast<EOnlineDataAdvertisementType::Type>(2));

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UArrowGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("세션 생성 최종 성공!"));
        GetWorld()->ServerTravel("/Game/ArrowGame/Maps/Map_Test?listen");
    }
}

void UArrowGameInstance::FindServer()
{
    UE_LOG(LogTemp, Log, TEXT("세션 검색 시작"));
    if (!SessionInterface.IsValid()) return;
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 10000; // 스팀이 자르더라도 요청은 최대로 둡니다.

    // 1. [기본] 로비 & 프레즌스 활성화 (이건 필수)
    SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    // 2. [강력한 필터] 빈자리가 1개 이상 있는 방만 가져와라!
    // Spacewar 방의 90%는 빈자리가 없거나 꽉 찬 유령 방입니다. 이걸로 엄청나게 걸러집니다.
    SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

    // 3. [승래님 고유 키] (이미 적용됨)
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
            if (auto* PC = GetFirstLocalPlayerController())
            {
                PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("세션 참가 실패"));
    }
}

void UArrowGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("스팀 오버레이 참가 요청 확인! 조인을 시작합니다."));
        
        // 찾아온 방 정보(InviteResult)를 가지고 기존에 만들어둔 Join 로직을 실행합니다.
        SessionInterface->JoinSession(0, NAME_GameSession, InviteResult);
    }
}