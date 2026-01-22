// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/ArrowCharacter.h"
#include "../Core/ArrowGameGameMode.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// 컴포넌트도 복제 설정 필요함
	SetIsReplicatedByDefault(true);

	// ...
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Health 변수를 복제 목록에 등록
	DOREPLIFETIME(UHealthComponent, Health);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// 체력 초기화는 권한이 있는 서버에서만 수행해도 되지만
	// 초기값 동기화를 위해 둘 다 해도 무방 (서버 값으로 덮어씌워짐)
	if (GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
	}

	// OnTakeAnyDamage �̺�Ʈ ���
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTaken);

	//GameMode ĳ���� �׽�Ʈ
	ArrowGameGameMode = Cast<AArrowGameGameMode>(UGameplayStatics::GetGameMode(this));
	if (ArrowGameGameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode found: %s"), *ArrowGameGameMode->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode cast failed!"));
	}
	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 서버전용 여기서만 게임 규칙(체력 깎기, 사망 판정)을 처리
void UHealthComponent::DamageTaken(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	class  AController* Instigator,
	AActor* DamageCause)
{
	if (Damage <= 0.f) return;

	// 서버 권한 확인!  클라이언트는 스스로 체력을 깎을 수 없게
	if (!GetOwner()->HasAuthority()) return;

	Health -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("%s Health: %.1f"), *GetOwner()->GetName(), Health);

	// 서버에서는 OnRep 함수가 자동 호출되지 않으니 서버도 반응이 필요하면 직접 호출
	OnRep_Health();

	// 사망 처리는 서버가 결정하고 GameMode에게 알림
	if (Health <= 0.f && ArrowGameGameMode)
	{
		ArrowGameGameMode->ActorDied(DamagedActor);
	}
	// OnRep_Health는 클라에서도 실행되는데, 게임모드는 이제 서버에만 있음..
}


// [클라이언트 + 서버] 여기서는 보여주는 것(Visual)만 처리
void UHealthComponent::OnRep_Health()
{
	//여기서 피격애니나 체력바 하면 될듯

	// 피격 애니메이션 재생
	AArrowCharacter* ArrowChar = Cast<AArrowCharacter>(GetOwner());
	if (ArrowChar)
	{
		// 죽은 상태가 아닐 때만 피격 모션 (죽었는데 피격 모션 나오면 이상하니까)
		if (Health > 0.f && ArrowChar->HitMontage)
		{
			ArrowChar->PlayMontage(ArrowChar->HitMontage);
		}
	}

	// 여기서 체력바 UI 갱신 코드를 넣을 수도 있음
	// ex) ArrowChar->UpdateHealthBar(Health);

	// 여기서 GameMode->ActorDied()를 호출하면 안 됨
	// 클라이언트는 GameMode를 모름 (NULL)
	// 죽었다는 판정은 서버가 이미 DamageTaken에서 했음

}