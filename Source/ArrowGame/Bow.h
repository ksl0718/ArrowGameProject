#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.h"
#include "Bow.generated.h"


class AArrowProjectile;

UENUM(BlueprintType)
enum class EBowState : uint8
{
    Idle      UMETA(DisplayName = "Idle"),
    Aim       UMETA(DisplayName = "Aim"),
    Charging  UMETA(DisplayName = "Charging")
};


UCLASS()
class ARROWGAME_API ABow : public AWeapon
{
    GENERATED_BODY()

public:
    // 기본 생성자
    ABow();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // 상태 함수
    virtual void StartAim() override;
    virtual void StopAim() override;
    virtual void StartDraw() override;
    virtual void EndDraw() override;

    // Bow 상태 조회 (BP에서도 가능)
    UFUNCTION(BlueprintCallable)
    bool IsAiming() const { return bIsAiming; }

    UFUNCTION(BlueprintCallable)
    bool IsCharging() const { return bIsCharging; }


    UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
    EBowState BowState = EBowState::Idle;

    UPROPERTY()
    AArrowProjectile* PreparedArrow = nullptr;

protected:
   
    // 조준 / 차징 상태
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State")
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State")
    bool bIsCharging = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State")
    float ChargeTime = 0.f;

    // 차징 관련 옵션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Charge")
    float MaxChargeTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Charge")
    float TiredThreshold = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Charge")
    float AutoReleaseTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Fire")
    float MinArrowSpeed = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Fire")
    float MaxArrowSpeed = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Projectile")
    TSubclassOf<AArrowProjectile> ArrowProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* DrawSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* FireSound;

private:
    void HandleCharge(float DeltaTime);
    void FireArrow(float Power);

    
};