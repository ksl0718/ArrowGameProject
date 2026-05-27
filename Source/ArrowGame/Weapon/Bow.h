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

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // ���� �Լ�
    virtual void StartAim() override;
    virtual void StopAim() override;
    
    //서버에 요청만
    virtual void StartDraw() override;
    virtual void EndDraw() override;
    
    void CancelAction();
    
    UFUNCTION(BlueprintCallable)
    bool IsCharging() const { return bIsCharging; }
    FORCEINLINE bool IsReloading() const { return bIsReloading; }
    FORCEINLINE bool HasPreparedArrow() const { return PreparedArrow != nullptr; }
    FORCEINLINE bool IsNocking() const { return bIsNocking; }
    FORCEINLINE float GetChargeTime() const { return ChargeTime; }
    FORCEINLINE float GetMaxChargeTime() const { return MaxChargeTime; }
    FORCEINLINE float GetTiredThreshold() const { return TiredThreshold; }
    FORCEINLINE bool IsPendingDraw() const { return bPendingDraw; }
    
    UPROPERTY(BlueprintReadOnly, Category = "Bow|State", Replicated)
    EBowState BowState = EBowState::Idle;

    UPROPERTY()
    AArrowProjectile* PreparedArrow = nullptr;

protected:
    /** Original BP flow: initial spawn socket on character mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Visual")
    FName InitialArrowSpawnSocketName = FName(TEXT("thigh_twist_01_r"));

    /** Original BP flow: first attach socket on character mesh (hand/finger). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Visual")
    FName ArrowHandSocketName = FName(TEXT("soc_index_03_r"));

    /** Original BP flow: final attach socket on bow mesh string. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Visual")
    FName BowStringSocketName = FName(TEXT("Socket_Bow_String"));

    /** Delay before moving nocked arrow from hand to bow string. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Visual")
    float NockToStringDelay = 0.2f;

    UFUNCTION(Server, Reliable)
    void ServerStartAim();

   //서버한테 활 당기라고 요청
    UFUNCTION(Server, Reliable)
    void ServerStartDraw();

    //활쏘라고 요청
    UFUNCTION(Server, Reliable)
    void ServerEndDraw();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayDrawSound();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayFireSound();
    
    UPROPERTY(ReplicatedUsing=OnRep_IsCharging, VisibleAnywhere, BlueprintReadOnly, Category = "Bow|State")
    bool bIsCharging = false;
    
    UPROPERTY(ReplicatedUsing = OnRep_IsVisualAiming)
    bool bIsVisualAiming = false;
    
    void UpdateArrowVisual();
    
    // RepNotify 콜백 함수 선언
    UFUNCTION()
    void OnRep_IsCharging();
    
    UFUNCTION()
    void OnRep_IsVisualAiming();
    
    // 헬퍼 함수들
    void SpawnDrawArrow();
    void DestroyDrawArrow();
    
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
    float MinArrowSpeed = 4000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Fire")
    float MaxArrowSpeed = 12000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow|Projectile")
    TSubclassOf<AArrowProjectile> ArrowProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* DrawSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* FireSound;

    // 발사 직후 재장전 중인지 확인하는 변수
    UPROPERTY(ReplicatedUsing = OnRep_IsReloading, BlueprintReadOnly, Category = "Bow|State")
    bool bIsReloading = false;
    
    UFUNCTION()
    void OnRep_IsReloading();
    
    UFUNCTION()
    void OnRep_IsNocking();
    
    // 화살을 시위에 거는 중인지 확인 (조준 진입 초기 단계)
    UPROPERTY(ReplicatedUsing = OnRep_IsNocking, BlueprintReadOnly, Category = "Bow|State")
    bool bIsNocking = false;

    // 장전 동작(등에서 꺼내기)에 걸리는 시간 (0.3~0.5초 추천)
    UPROPERTY(EditAnywhere, Category = "Bow|Fire")
    float NockingDelayTime = 0.15f;
    
    // 발사 후 다음 화살을 꺼낼 때까지 걸리는 시간 (0.4~0.7초 추천)
    UPROPERTY(EditAnywhere, Category = "Bow|Fire")
    float ReloadDelayTime = 0.6f;

    // 쿨타임이 끝나면 호출될 함수
    void FinishReloading();
    
    void FinishNocking();
    
    bool bPendingDraw = false;

    FTimerHandle ReloadTimerHandle;
    FTimerHandle NockingTimerHandle;
    FTimerHandle NockToStringTimerHandle;
private:
    UPROPERTY(EditAnywhere, Category = "Mesh")
    class USkeletalMeshComponent* Mesh;
    
    void HandleCharge(float DeltaTime);
    void AttachPreparedArrowToBowString();
    
    void FireArrow(float Power);

    
};