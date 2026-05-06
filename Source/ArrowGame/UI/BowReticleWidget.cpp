#include "BowReticleWidget.h"
#include "ArrowGame/Character/UserArcherCharacter.h"
#include "ArrowGame/Weapon/Bow.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"

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
		if (ReticleDMI)
		{
			ReticleDMI->SetVectorParameterValue(TEXT("ReticleColor"), FLinearColor(0.f, 1.f, 0.f));
		}
	}
}

void UBowReticleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ReticleDMI) return;

	ReticleDMI->SetScalarParameterValue(TEXT("ChargeAlpha"), GetChargeAlpha());

	bool bNewEnemyAimed = CheckEnemyAimed();
	if (bNewEnemyAimed != bEnemyAimed)
	{
		bEnemyAimed = bNewEnemyAimed;
		FLinearColor Color = bEnemyAimed ? FLinearColor::Red : FLinearColor(0.f, 1.f, 0.f);
		ReticleDMI->SetVectorParameterValue(TEXT("ReticleColor"), Color);
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

bool UBowReticleWidget::CheckEnemyAimed() const
{
	if (!OwnerCharacter.IsValid()) return false;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return false;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	// 벽 먼저 체크 (Visibility)
	FHitResult WallHit;
	GetWorld()->LineTraceSingleByChannel(WallHit, CamLoc, CamLoc + CamRot.Vector() * 10000.f, ECC_Visibility, Params);
	float MaxDist = WallHit.bBlockingHit ? WallHit.Distance : 10000.f;

	// 캐릭터 감지 (Pawn 오브젝트 타입)
	FHitResult Hit;
	FCollisionObjectQueryParams ObjParams(ECC_Pawn);
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, CamLoc, CamLoc + CamRot.Vector() * MaxDist, ObjParams, Params))
		return false;

	APawn* HitPawn = Cast<APawn>(Hit.GetActor());
	if (!HitPawn) return false;

	AArrowPlayerState* MyPS = OwnerCharacter->GetPlayerState<AArrowPlayerState>();
	AArrowPlayerState* HitPS = HitPawn->GetPlayerState<AArrowPlayerState>();
	if (!MyPS || !HitPS) return false;

	return MyPS->IsDokkaebi() != HitPS->IsDokkaebi();
}
