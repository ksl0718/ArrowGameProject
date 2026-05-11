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

	MulticastSpawnExplosionFX(Hit.ImpactPoint);

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

	// 즉시 Destroy 대신 지연 — 멀티캐스트 RPC가 클라에 도달할 시간 확보
	SetActorHiddenInGame(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(1.f);
}

void AExplosiveArrow::MulticastSpawnExplosionFX_Implementation(FVector Location)
{
	if (ExplosionFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionFX, Location);
	if (ExplosionSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Location);
}
