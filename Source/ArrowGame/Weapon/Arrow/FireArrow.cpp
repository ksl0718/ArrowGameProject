#include "FireArrow.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ArrowGame/Component/HealthComponent.h"
#include "Sound/SoundBase.h"

AFireArrow::AFireArrow()
{
}

void AFireArrow::NotifyImpact(const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	MulticastSpawnFireFX(GetActorLocation());

	TArray<FHitResult> OutHits;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(BurnRadius);

	GetWorld()->SweepMultiByChannel(OutHits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere);

	for (auto& HitResult : OutHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || !IsEnemy(GetInstigator(), HitActor)) continue;

		UHealthComponent* HC = HitActor->FindComponentByClass<UHealthComponent>();
		if (HC) HC->StartBurn(BurnDuration, BurnInterval, BurnDamage);
	}
}

void AFireArrow::MulticastSpawnFireFX_Implementation(FVector Location)
{
	if (FireFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireFX, Location);
	if (FireImpactSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireImpactSound, Location);
}
