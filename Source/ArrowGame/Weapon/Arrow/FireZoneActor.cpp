#include "FireZoneActor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
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

	FireLoopAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireLoopAudio"));
	FireLoopAudio->SetupAttachment(BurnSphere);
	FireLoopAudio->bAutoActivate = false;
	FireLoopAudio->bAllowSpatialization = true;
	FireLoopAudio->bOverrideAttenuation = true;
}

void AFireZoneActor::InitZone(APawn* InInstigator, float InRadius, float InLifetime,
	float InBurnDamage, float InBurnInterval, float InBurnDuration,
	UNiagaraSystem* InGroundFX, UNiagaraSystem* InBodyBurnFX,
	USoundBase* InLoopSound)
{
	DamageInstigator = InInstigator;
	BurnDamage = InBurnDamage;
	BurnInterval = InBurnInterval;
	BurnDuration = InBurnDuration;
	BodyBurnFX = InBodyBurnFX;
	ZoneLifetime = InLifetime;
	GroundFireLoopSound = InLoopSound;

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

	StartFireLoopSound();
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

void AFireZoneActor::StartFireLoopSound()
{
	if (!FireLoopAudio || !GroundFireLoopSound)
	{
		return;
	}

	FireLoopAudio->SetSound(GroundFireLoopSound);
	if (FireSoundAttenuation)
	{
		FireLoopAudio->AttenuationSettings = FireSoundAttenuation;
	}
	FireLoopAudio->Play();
}

void AFireZoneActor::StopFireLoopSound()
{
	if (FireLoopAudio && FireLoopAudio->IsPlaying())
	{
		FireLoopAudio->Stop();
	}
}

void AFireZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFireLoopSound();

	if (HasAuthority() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnTickHandle);
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

	for (APawn* Pawn : PawnsInRadius)
	{
		ApplyBurnToActor(Pawn);
	}
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

	// 불판 안에 있는 동안 BurnInterval마다 StartBurn → Duration(도트 틱 수) 갱신, 몸 불 FX 유지
	HC->StartBurn(BurnDuration, BurnInterval, BurnDamage, InstigatorController, BodyBurnFX);
}
