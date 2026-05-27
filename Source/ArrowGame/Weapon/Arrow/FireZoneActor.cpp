#include "FireZoneActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "ArrowGame/Component/HealthComponent.h"

AFireZoneActor::AFireZoneActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BurnSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BurnSphere"));
	SetRootComponent(BurnSphere);
	BurnSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BurnSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	BurnSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BurnSphere->SetGenerateOverlapEvents(true);
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
}

void AFireZoneActor::BeginPlay()
{
	Super::BeginPlay();

	if (GroundFireNiagara && GroundFireNiagara->GetAsset())
	{
		GroundFireNiagara->Activate(true);
	}

	SetLifeSpan(ZoneLifetime);

	if (!HasAuthority()) return;

	BurnSphere->OnComponentBeginOverlap.AddDynamic(this, &AFireZoneActor::OnBurnSphereBeginOverlap);
	BurnSphere->OnComponentEndOverlap.AddDynamic(this, &AFireZoneActor::OnBurnSphereEndOverlap);

	GetWorld()->GetTimerManager().SetTimer(
		BurnTickHandle,
		this,
		&AFireZoneActor::TickBurnZone,
		BurnInterval,
		true
	);

	TArray<AActor*> AlreadyOverlapping;
	BurnSphere->GetOverlappingActors(AlreadyOverlapping, APawn::StaticClass());
	for (AActor* Actor : AlreadyOverlapping)
	{
		OnBurnSphereBeginOverlap(BurnSphere, Actor, nullptr, 0, false, FHitResult());
	}
}

void AFireZoneActor::OnBurnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || !OtherActor->IsA(APawn::StaticClass())) return;

	OverlappingBurnTargets.Add(OtherActor);
	ApplyBurnToActor(OtherActor);
}

void AFireZoneActor::OnBurnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingBurnTargets.Remove(OtherActor);
	}
}

void AFireZoneActor::TickBurnZone()
{
	if (!HasAuthority()) return;

	for (auto It = OverlappingBurnTargets.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			continue;
		}
		ApplyBurnToActor(It->Get());
	}
}

void AFireZoneActor::ApplyBurnToActor(AActor* Target)
{
	if (!Target) return;

	UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>();
	if (!HC) return;

	AController* InstigatorController = DamageInstigator.IsValid()
		? DamageInstigator->GetController()
		: nullptr;

	HC->StartBurn(BurnDuration, BurnInterval, BurnDamage, InstigatorController, BodyBurnFX);
}
