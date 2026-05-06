#include "BowReticleWidget.h"
#include "ArrowGame/Character/UserArcherCharacter.h"
#include "ArrowGame/Weapon/Bow.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UBowReticleWidget::InitReticle(AUserArcherCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
}

void UBowReticleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReticleImage)
	{
		ReticleDMI = ReticleImage->GetDynamicMaterial();
	}
}

void UBowReticleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ReticleDMI)
	{
		ReticleDMI->SetScalarParameterValue(TEXT("ChargeAlpha"), GetChargeAlpha());
	}
}

float UBowReticleWidget::GetChargeAlpha() const
{
	ABow* Bow = GetBow();
	if (!Bow || !Bow->IsCharging()) return 0.f;
	return FMath::Clamp(Bow->GetChargeTime() / Bow->GetMaxChargeTime(), 0.f, 1.f);
}

float UBowReticleWidget::GetReticleRadius() const
{
	return FMath::Lerp(MaxRadius, MinRadius, GetChargeAlpha());
}

bool UBowReticleWidget::GetIsAiming() const
{
	return OwnerCharacter.IsValid() && OwnerCharacter->IsAiming();
}

ABow* UBowReticleWidget::GetBow() const
{
	if (!OwnerCharacter.IsValid()) return nullptr;
	return Cast<ABow>(OwnerCharacter->GetEquippedWeapon());
}
