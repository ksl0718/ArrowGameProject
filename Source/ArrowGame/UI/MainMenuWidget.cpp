#include "MainMenuWidget.h"

#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	// 서버 방 만들기 로직
	RemoveFromParent();
	
	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::OpenLevel(World, TEXT("ThirdPersonMap"), true, TEXT("listen"));
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	if (!IPAddressInput || !Txt_Status) return;
	FString IP = IPAddressInput->GetText().ToString();
	
	if (IP.IsEmpty()) {
		Txt_Status->SetText(FText::FromString(TEXT("IP 주소를 입력해주세요.")));
		return; // [추가]
	}
	if (!IsValidIP(IP)) {
		Txt_Status->SetText(FText::FromString(TEXT("유효하지 않은 IP 형식입니다.")));
		return; // [추가]
	}
	
	Txt_Status->SetText(FText::FromString(TEXT("서버에 접속 중...")));
	Txt_Status->SetColorAndOpacity(FLinearColor::Green);
    
	UGameplayStatics::OpenLevel(GetWorld(), FName(*IP));
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