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
    // �⺻ ������
    ABow();

    // 변수 동기화 규칙을 정의하는 필수 함수
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // ���� �Լ�
    virtual void StartAim() override;
    virtual void StopAim() override;

    //얘낸 이제 요청만
    virtual void StartDraw() override;
    virtual void EndDraw() override;

    // Bow ���� ��ȸ (BP������ ����)
    UFUNCTION(BlueprintCallable)
    bool IsAiming() const { return bIsAiming; }

    UFUNCTION(BlueprintCallable)
    bool IsCharging() const { return bIsCharging; }


    UPROPERTY(BlueprintReadOnly, Category = "Bow|State", Replicated)
    EBowState BowState = EBowState::Idle;

    // 화살 객체는 서버가 만들어서 클라이언트에게 알려줘야 하니 Replicated
    // 이거 나중에 protected로 캡슐화 하고 public에 확인용 함수 추가필요
    UPROPERTY(Replicated) 
    AArrowProjectile* PreparedArrow = nullptr;

protected:

    // 서버한테 활 당기기 시작해라고 명령하는 함수
    // Reliable: 게임플레이에 중요하므로 반드시 도착해야 함
    UFUNCTION(Server, Reliable)
    void ServerStartDraw();

    // 서버한테 활 쏴! 명령
    UFUNCTION(Server, Reliable)
    void ServerEndDraw();

    
    
    // ���� / ��¡ ����
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State", Replicated)
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State", Replicated)
    bool bIsCharging = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State", Replicated)
    float ChargeTime = 0.f;

    // ��¡ ���� �ɼ�
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