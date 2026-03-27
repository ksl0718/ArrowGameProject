#pragma once

#include "CoreMinimal.h"
#include "ArrowGame/Weapon/ArrowProjectile.h"
#include "FireArrow.generated.h"

UCLASS()
class ARROWGAME_API AFireArrow : public AArrowProjectile
{
	GENERATED_BODY()

public:
	AFireArrow();

protected:

	virtual void NotifyImpact(const FHitResult& Hit) override;
	
	// 불화살만의 전용 변수들
	UPROPERTY(EditAnywhere, Category = "Fire")
	float BurnDamage = 5.f;

	UPROPERTY(EditAnywhere, Category = "Fire")
	float BurnDuration = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
	class UNiagaraSystem* FireFX;
};