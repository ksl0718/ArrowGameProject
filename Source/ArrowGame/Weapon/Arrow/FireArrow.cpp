#include "FireArrow.h"
#include "FireZoneActor.h"
#include "Kismet/GameplayStatics.h"
#include "ArrowGame/Component/HealthComponent.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"

AFireArrow::AFireArrow()
{
	ArrowType = EArrowType::Fire;

	TipNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TipFX"));
	TipNiagara->SetupAttachment(ArrowMesh);
	TipNiagara->bAutoActivate = false;
}

void AFireArrow::PlayLaunchEffects()
{
	if (LaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), LaunchSound, GetActorLocation());
	}
}

void AFireArrow::NotifyImpact(const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	AActor* HitActor = Hit.GetActor();
	const bool bHitPawn = HitActor && HitActor->IsA(APawn::StaticClass());

	if (bHitPawn)
	{
		ApplyBurnToPawn(HitActor);
		MulticastPlayFireImpactSound(Hit.ImpactPoint);
	}
	else
	{
		SpawnFireZone(Hit);
	}
}

void AFireArrow::ApplyBurnToPawn(AActor* Target)
{
	if (!Target) return;

	if (UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>())
	{
		HC->StartBurn(BurnDuration, BurnInterval, BurnDamage, GetInstigatorController(), BodyBurnFX);
	}
}

void AFireArrow::SpawnFireZone(const FHitResult& Hit)
{
	if (!FireZoneClass) return;

	const FVector SpawnLocation = Hit.ImpactPoint + Hit.ImpactNormal * 5.f;
	const FRotator SpawnRotation = Hit.ImpactNormal.Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFireZoneActor* Zone = GetWorld()->SpawnActor<AFireZoneActor>(FireZoneClass, SpawnLocation, SpawnRotation, Params);
	if (!Zone) return;

	Zone->InitZone(
		GetInstigator(),
		GroundFireRadius,
		GroundFireLifetime,
		BurnDamage,
		BurnInterval,
		BurnDuration,
		GroundFireFX,
		BodyBurnFX,
		GroundFireLoopSound
	);
}

void AFireArrow::MulticastPlayFireImpactSound_Implementation(FVector Location)
{
	if (FireImpactSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FireImpactSound, Location);
	}
}

