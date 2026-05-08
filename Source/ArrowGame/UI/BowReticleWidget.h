#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BowReticleWidget.generated.h"

class AUserArcherCharacter;
class ABow;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class ARROWGAME_API UBowReticleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitReticle(AUserArcherCharacter* InOwnerCharacter);

	/** 0.0 = 차징 전 (최대 원), 1.0 = 풀 차징 (점) */
	UFUNCTION(BlueprintPure, Category = "Reticle")
	float GetChargeAlpha() const;

	/** 레티클 원의 반지름(px). SizeBox 크기 바인딩에 사용 */
	UFUNCTION(BlueprintPure, Category = "Reticle")
	float GetReticleRadius() const;

	/** 조준 중이고 활이 유효한지 */
	UFUNCTION(BlueprintPure, Category = "Reticle")
	bool GetIsAiming() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reticle")
	float MaxRadius = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reticle")
	float MinRadius = 3.f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** WBP에서 이름이 "ReticleImage"인 Image 위젯에 자동 바인딩 */
	UPROPERTY(BlueprintReadOnly, Category = "Reticle", meta = (BindWidgetOptional))
	UImage* ReticleImage = nullptr;

private:
	TWeakObjectPtr<AUserArcherCharacter> OwnerCharacter;
	UPROPERTY()
	UMaterialInstanceDynamic* ReticleDMI = nullptr;

	ABow* GetBow() const;
	bool CheckEnemyAimed() const;

	bool bEnemyAimed = false;
	float EnemyCheckAccumulator = 0.f;
	static constexpr float EnemyCheckInterval = 0.1f;
};
