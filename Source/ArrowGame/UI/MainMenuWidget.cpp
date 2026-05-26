#include "MainMenuWidget.h"

#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../ArrowGameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
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

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	// 버튼 클릭 이벤트 연결
	if (Btn_Host)
	{
		Btn_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHostClicked);
	}

	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinClicked);
	}
	
	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnHostClicked()
{
	if (UWorld* World = GetWorld())
	{
		if (Txt_Status)
		{
			Txt_Status->SetText(FText::FromString(TEXT("로비로 이동 중...")));
			Txt_Status->SetColorAndOpacity(FLinearColor::Green);
		}

		// 메인 메뉴에서 CreateSession 후 ServerTravel 하면 스팀 초대 URL이 MainMenuMap으로 남음.
		// 로비에서 listen 서버를 띄운 뒤 세션을 등록해야 초대/조인 클라이언트가 LobbyMap으로 붙음.
		UGameplayStatics::OpenLevel(
			World,
			FName(TEXT("/Game/ArrowGame/Maps/LobbyMap")),
			true,
			TEXT("listen?game=/Script/ArrowGame.LobbyGameMode"));
		RemoveFromParent();
		return;
	}

	if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(GetGameInstance()))
	{
		if (Txt_Status)
		{
			Txt_Status->SetText(FText::FromString(TEXT("방 생성 중...")));
			Txt_Status->SetColorAndOpacity(FLinearColor::Green);
		}
		ArrowGI->CreateServer();
		RemoveFromParent();
	}
	else if (Txt_Status)
	{
		Txt_Status->SetText(FText::FromString(TEXT("GameInstance를 찾을 수 없습니다.")));
		Txt_Status->SetColorAndOpacity(FLinearColor::Red);
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	if (!Txt_Status) return;

	if (UArrowGameInstance* ArrowGI = Cast<UArrowGameInstance>(GetGameInstance()))
	{
		Txt_Status->SetText(FText::FromString(TEXT("방 검색 중...")));
		Txt_Status->SetColorAndOpacity(FLinearColor::Green);
		ArrowGI->FindServer();
	}
	else
	{
		Txt_Status->SetText(FText::FromString(TEXT("GameInstance를 찾을 수 없습니다.")));
		Txt_Status->SetColorAndOpacity(FLinearColor::Red);
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(
			GetWorld(), 
			GetOwningPlayer(), 
			EQuitPreference::Quit, 
			false // Ignore Platform Settings
		);
}

bool UMainMenuWidget::IsValidIP(const FString& IP)
{
	// IPv4 정규표현식 패턴
	const FString IPPattern = TEXT("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
	FRegexPattern Pattern(IPPattern);
	FRegexMatcher Matcher(Pattern, IP);

	return Matcher.FindNext();
}