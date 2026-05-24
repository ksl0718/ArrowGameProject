#include "FireArrow.h"
#include "FireZoneActor.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ArrowGame/Component/HealthComponent.h"
#include "Sound/SoundBase.h"

AFireArrow::AFireArrow()
{
	ArrowType = EArrowType::Fire;
}

void AFireArrow::NotifyImpact(const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	AActor* HitActor = Hit.GetActor();
	const bool bHitPawn = HitActor && HitActor->IsA(APawn::StaticClass());

	if (bHitPawn)
	{
		MulticastSpawnFireFX(Hit.ImpactPoint, HitActor);

		TArray<FHitResult> OutHits;
		const FVector Origin = GetActorLocation();
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(BurnRadius);
		GetWorld()->SweepMultiByChannel(OutHits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere);

		for (const FHitResult& HitResult : OutHits)
		{
			AActor* BurnTarget = HitResult.GetActor();
			if (!BurnTarget || !IsEnemy(GetInstigator(), BurnTarget)) continue;

			if (UHealthComponent* HC = BurnTarget->FindComponentByClass<UHealthComponent>())
			{
				HC->StartBurn(BurnDuration, BurnInterval, BurnDamage);
			}
		}
	}
	else
	{
		SpawnFireZone(Hit);
		MulticastPlayFireImpactSound(Hit.ImpactPoint);
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

	UNiagaraSystem* FX = GroundFireFX ? GroundFireFX : FireFX;
	Zone->InitZone(GroundFireRadius, GroundFireLifetime, FX);
}

void AFireArrow::MulticastSpawnFireFX_Implementation(FVector Location, AActor* AttachTarget)
{
	if (FireFX)
	{
		if (AttachTarget && AttachTarget->GetRootComponent())
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				FireFX,
				AttachTarget->GetRootComponent(),
				NAME_None,
				Location,
				FRotator::ZeroRotator,
				EAttachLocation::KeepWorldPosition,
				true
			);
		}
	}
	if (FireImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireImpactSound, Location);
	}
}

void AFireArrow::MulticastPlayFireImpactSound_Implementation(FVector Location)
{
	if (FireImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireImpactSound, Location);
	}
}
