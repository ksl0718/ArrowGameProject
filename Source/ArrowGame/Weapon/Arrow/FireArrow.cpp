#include "FireArrow.h"
#include "NiagaraFunctionLibrary.h"
#include "ArrowGame/Component/HealthComponent.h"

AFireArrow::AFireArrow()
{
	BurnDamage = 10.f;
	BurnDuration = 5.f;
}

void AFireArrow::NotifyImpact(const FHitResult& Hit)
{
	SpawnFireFX();
	
	TArray<FHitResult> OutHits;
	FVector SweepStart = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
	
	bool bHasHit = GetWorld()->SweepMultiByChannel(OutHits, SweepStart,SweepStart, FQuat::Identity,ECC_Pawn,Sphere);
	
	if (bHasHit)
	{
		for (auto& HitResult : OutHits)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				UHealthComponent* HC = HitActor->FindComponentByClass<UHealthComponent>();
				if (HC)
				{
					HC->StartBurn(BurnDuration,BurnInterval,BurnDamage);
				}
			}
		}
	}
	Super::NotifyImpact(Hit);
}

void AFireArrow::SpawnFireFX()
{
	if (FireFX)
	{
		// #include "NiagaraFunctionLibrary.h" 필수!
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			FireFX, 
			GetActorLocation()
		);
	}
}
