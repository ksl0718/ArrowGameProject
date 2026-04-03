#include "ExplosiveArrow.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


AExplosiveArrow::AExplosiveArrow()
{
	// 1. 생성자에서 화살촉 데미지 스위치 꺼버리기
	bShouldApplyDirectDamage = false;
    
	ExplosionDamage = 50.f;
	ExplosionRadius = 300.f;
}

void AExplosiveArrow::NotifyImpact(const FHitResult& Hit)
{
	// 부모의 NotifyImpact는 비어있으니 안 불러도 되지만, 습관적으로 Super:: 호출해도 좋습니다.
	Super::NotifyImpact(Hit);

	// 2. 서버에서 범위 데미지 처리
	if (HasAuthority())
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		IgnoreActors.Add(GetInstigator());

		UGameplayStatics::ApplyRadialDamage(
			this,
			ExplosionDamage,
			GetActorLocation(),
			ExplosionRadius,
			UDamageType::StaticClass(),
			IgnoreActors,
			this,
			GetInstigatorController(),
			true // 중심점일수록 풀 데미지
		);
	}

	// 3. 시각 효과 (Niagara) - 어디든 맞으면 펑!
	if (ExplosionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	}

	// 4. 폭발 화살은 벽에 박힐 필요 없이 바로 사라져야 함 (서버에서)
	if (HasAuthority())
	{
		Destroy();
	}
}