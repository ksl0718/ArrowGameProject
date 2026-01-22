// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArrowCharacter.h"
#include "InputActionValue.h"
#include "../Weapon/Bow.h"
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

	bool IsDead() const { return bIsDead; }
    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bCanMove = true;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* WalkAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* RollAction;


    // ī�޶�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

   

    // FOV ������
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float NormalFOV = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float AimFOV = 65.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float AimInterpSpeed = 15.f;

    //�̵� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float NormalSpeed = 400.f;     // �⺻ �ӵ�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 200.f;       // ����Ʈ ���� �� ���� �ӵ�
    

    
    // �̵� / �þ� / ���� / �߻� �Լ�
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAiming();
    void StopAiming();
    void StartCharging();           // LMB ������ ��
    void ReleaseArrow();            // LMB ���� ��
    void OnWalkSlowStarted(const FInputActionValue& Value);
    void OnWalkSlowEnded(const FInputActionValue& Value);
    void Roll(); // ������

    UFUNCTION(BlueprintCallable)
	void OnRollEnd(); // ������ �ִϸ��̼� ������ �� ȣ��

    virtual void OnDeath() override;
};
