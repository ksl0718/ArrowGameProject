#include "FireZoneActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AFireZoneActor::AFireZoneActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BurnSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BurnSphere"));
	SetRootComponent(BurnSphere);
	BurnSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BurnSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	BurnSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BurnSphere->SetGenerateOverlapEvents(false);

	GroundFireNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GroundFire"));
	GroundFireNiagara->SetupAttachment(BurnSphere);
	GroundFireNiagara->bAutoActivate = false;
}

void AFireZoneActor::InitZone(float InRadius, float InLifetime, UNiagaraSystem* InGroundFX)
{
	if (BurnSphere)
	{
		BurnSphere->SetSphereRadius(InRadius);
	}

	ZoneLifetime = InLifetime;

	if (GroundFireNiagara && InGroundFX)
	{
		GroundFireNiagara->SetAsset(InGroundFX);
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
}
