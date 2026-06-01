#include "FireZoneActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "ArrowGame/Component/HealthComponent.h"
#include "Engine/OverlapResult.h"

AFireZoneActor::AFireZoneActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BurnSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BurnSphere"));
	SetRootComponent(BurnSphere);
	// 화상 판정은 GatherPawnsInBurnRadius 수동 오버랩만 사용 — Block 하면 화살이 존에 박힘
	BurnSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BurnSphere->SetGenerateOverlapEvents(false);
	BurnSphere->SetHiddenInGame(true, true);

	GroundFireNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GroundFire"));
	GroundFireNiagara->SetupAttachment(BurnSphere);
	GroundFireNiagara->bAutoActivate = false;
}

void AFireZoneActor::InitZone(APawn* InInstigator, float InRadius, float InLifetime,
	float InBurnDamage, float InBurnInterval, float InBurnDuration,
	UNiagaraSystem* InGroundFX, UNiagaraSystem* InBodyBurnFX)
{
	DamageInstigator = InInstigator;
	BurnDamage = InBurnDamage;
	BurnInterval = InBurnInterval;
	BurnDuration = InBurnDuration;
	BodyBurnFX = InBodyBurnFX;
	ZoneLifetime = InLifetime;

	if (BurnSphere)
	{
		BurnSphere->SetSphereRadius(InRadius);
	}

	if (GroundFireNiagara && InGroundFX)
	{
		GroundFireNiagara->SetAsset(InGroundFX);
	}

	if (HasAuthority())
	{
		TickBurnZone();
	}
}

void AFireZoneActor::GatherPawnsInBurnRadius(TArray<APawn*>& OutPawns) const
{
	OutPawns.Reset();

	if (!BurnSphere || !GetWorld())
	{
		return;
	}

	const FVector Center = BurnSphere->GetComponentLocation();
	const float Radius = BurnSphere->GetScaledSphereRadius();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FireZonePawnOverlap), false, this);
	QueryParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> OverlapResults;
	if (!GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams))
	{
		return;
	}

	TSet<APawn*> Unique;
	for (const FOverlapResult& Result : OverlapResults)
	{
		APawn* Pawn = Cast<APawn>(Result.GetActor());
		if (Pawn && !Unique.Contains(Pawn))
		{
			Unique.Add(Pawn);
			OutPawns.Add(Pawn);
		}
	}
}

void AFireZoneActor::BeginPlay()
{
	Super::BeginPlay();

	if (GroundFireNiagara && GroundFireNiagara->GetAsset())
	{
		GroundFireNiagara->Activate(true);
	}

	SetLifeSpan(ZoneLifetime);

	if (!HasAuthority())
	{
		return;
	}

	TickBurnZone();

	GetWorld()->GetTimerManager().SetTimer(
		BurnTickHandle,
		this,
		&AFireZoneActor::TickBurnZone,
		BurnInterval,
		true);
}

void AFireZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		for (const TWeakObjectPtr<APawn>& Pawn : PawnsInZoneLastTick)
		{
			if (Pawn.IsValid())
			{
				StopBurnOnActor(Pawn.Get());
			}
		}
		PawnsInZoneLastTick.Empty();

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(BurnTickHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AFireZoneActor::TickBurnZone()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<APawn*> PawnsInRadius;
	GatherPawnsInBurnRadius(PawnsInRadius);

	TSet<TWeakObjectPtr<APawn>> CurrentSet;
	for (APawn* Pawn : PawnsInRadius)
	{
		if (!Pawn)
		{
			continue;
		}

		CurrentSet.Add(Pawn);
		ApplyBurnToActor(Pawn);
	}

	for (const TWeakObjectPtr<APawn>& Previous : PawnsInZoneLastTick)
	{
		if (Previous.IsValid() && !CurrentSet.Contains(Previous))
		{
			StopBurnOnActor(Previous.Get());
		}
	}

	PawnsInZoneLastTick = MoveTemp(CurrentSet);
}

void AFireZoneActor::ApplyBurnToActor(APawn* Target)
{
	if (!Target)
	{
		return;
	}

	UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>();
	if (!HC)
	{
		return;
	}

	AController* InstigatorController = DamageInstigator.IsValid()
		? DamageInstigator->GetController()
		: nullptr;

	HC->ApplyZoneBurnTick(BurnDamage, InstigatorController, BodyBurnFX);
}

void AFireZoneActor::StopBurnOnActor(APawn* Target)
{
	if (!Target)
	{
		return;
	}

	if (UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>())
	{
		HC->StopZoneBurnEffects();
	}
}
