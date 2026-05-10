#include "ExplosiveArrow.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AExplosiveArrow::AExplosiveArrow()
{
	bShouldApplyDirectDamage = false;
}

void AExplosiveArrow::NotifyImpact(const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	MulticastSpawnExplosionFX(GetActorLocation());

	TArray<FHitResult> OutHits;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);

	GetWorld()->SweepMultiByChannel(OutHits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere);

	for (auto& HitResult : OutHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActor == GetInstigator()) continue;

		UGameplayStatics::ApplyDamage(HitActor, ExplosionDamage, GetInstigatorController(), this, UDamageType::StaticClass());
	}

	Destroy();
}

void AExplosiveArrow::MulticastSpawnExplosionFX_Implementation(FVector Location)
{
	if (ExplosionFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionFX, Location);
	if (ExplosionSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Location);
}
