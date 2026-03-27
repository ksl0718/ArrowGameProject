#include "FireArrow.h"
#include "NiagaraFunctionLibrary.h"

AFireArrow::AFireArrow()
{
	BurnDamage = 10.f;
	BurnDuration = 5.f;
}

void AFireArrow::NotifyImpact(const FHitResult& Hit)
{
	Super::NotifyImpact(Hit);
	
	if (FireFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireFX, GetActorLocation());
	}
	
	if (HasAuthority())
	{
		
	}
}
