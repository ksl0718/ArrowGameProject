// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArrowCharacter.h"
#include "InputActionValue.h"
#include "UserCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ARROWGAME_API AUserCharacter : public AArrowCharacter
{
	GENERATED_BODY()
public:
    AUserCharacter();

    void HandleDeath();

    bool IsAiming() const { return bIsAiming; }
    bool IsCharging() const { return bIsCharging; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // Enhanced Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ShootAction;

    // 카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    // 조준 상태
    bool bIsAiming = false;

    // FOV 보간용
    UPROPERTY(EditAnywhere, Category = "Camera")
    float NormalFOV = 90.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float AimFOV = 65.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float AimInterpSpeed = 15.f;

    //Charging 관련 변수
    bool bIsCharging = false;
    bool bIsTired = false;
    float ChargeTime = 0.f;

    UPROPERTY(EditAnywhere, Category = "Arrow|Charge")
    float MaxChargeTime = 1.0f;  // 최대 차징 시간

    UPROPERTY(EditAnywhere, Category = "Arrow|Charge")
    float TiredThreshold = 3.0f; // 힘든 구간

    UPROPERTY(EditAnywhere, Category = "Arrow|Charge")
    float AutoReleaseTime = 5.0f; // 강제 발시 구간

    UPROPERTY(EditAnywhere, Category = "Arrow|Charge")
    float MinArrowSpeed = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Arrow|Charge")
    float MaxArrowSpeed = 6000.f;

    // 이동 / 시야 / 조준 / 발사 함수
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAiming();
    void StopAiming();
    void StartCharging();           // LMB 눌렀을 때
    void ReleaseArrow();            // LMB 뗐을 때

    
    virtual void OnDeath() override;
};
