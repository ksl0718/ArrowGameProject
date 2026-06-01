#include "ExplosiveArrow.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AExplosiveArrow::AExplosiveArrow()
{
	ArrowType = EArrowType::Explosive;
	bShouldApplyDirectDamage = false;
	bAlwaysRelevant = true;

	TipNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TipFX"));
	TipNiagara->SetupAttachment(ArrowMesh);
	TipNiagara->bAutoActivate = false;
}

void AExplosiveArrow::NotifyImpact(const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	FVector Origin = Hit.ImpactPoint;
	MulticastSpawnExplosionFX(Origin);

	TArray<FHitResult> OutHits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);

	GetWorld()->SweepMultiByChannel(OutHits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere);

	for (auto& HitResult : OutHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) continue;

		UGameplayStatics::ApplyDamage(HitActor, ExplosionDamage, GetInstigatorController(), this, UDamageType::StaticClass());
	}

	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	SetLifeSpan(1.f);
}

void AExplosiveArrow::MulticastSpawnExplosionFX_Implementation(FVector Location)
{
	if (ExplosionFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionFX, Location);
	if (ExplosionSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Location);
}
