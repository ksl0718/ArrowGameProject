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

	AActor* HitActor = Hit.GetActor();
	bool bHitPawn = HitActor && HitActor->IsA(APawn::StaticClass());
	MulticastSpawnFireFX(Hit.ImpactPoint, bHitPawn ? HitActor : nullptr);

	TArray<FHitResult> OutHits;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(BurnRadius);

	GetWorld()->SweepMultiByChannel(OutHits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere);

	for (auto& HitResult : OutHits)
	{
		AActor* BurnTarget = HitResult.GetActor();
		if (!BurnTarget || !IsEnemy(GetInstigator(), BurnTarget)) continue;

		UHealthComponent* HC = BurnTarget->FindComponentByClass<UHealthComponent>();
		if (HC) HC->StartBurn(BurnDuration, BurnInterval, BurnDamage);
	}
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
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireFX, Location);
		}
	}
	if (FireImpactSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireImpactSound, Location);
}
