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
	
	bool IsDead() const { return bIsDead; }
    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bCanMove = true;
	
	void EquipNewBow(TSubclassOf<ABow> NewBowClass);
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* CycleArrowAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* InteractAction;
	
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
	
	
    // �̵� / �þ� / ���� / �߻� �Լ�
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAiming();
    void StopAiming();
    void StartCharging();           // LMB ������ ��
    void ReleaseArrow();            // LMB ���� ��
	
	void Input_CycleArrow(const FInputActionValue& Value); // 마우스휠 화살변경
	void Input_Interact(const FInputActionValue& Value); // F키 (상호작용) 입력 처리
	
    void OnWalkSlowStarted(const FInputActionValue& Value);
    void OnWalkSlowEnded(const FInputActionValue& Value);
	
	UFUNCTION(Server, Reliable)
	void ServerSetMaxWalkSpeed(float NewSpeed);
	
    void Roll(); // ������
	
	UFUNCTION(Server, Reliable)
	void ServerPlayRoll();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayRoll();
	
    UFUNCTION(BlueprintCallable)
	void OnRollEnd(UAnimMontage* Montage, bool bInterrupted); // ������ �ִϸ��̼� ������ �� ȣ��
	
	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* HitActor);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class USphereComponent* InteractionSphere; // 아이템 감지용 구체 컴포넌트
	
	UPROPERTY()
	TArray<AActor*> OverlappingActors;
	
	UPROPERTY()
	AActor* CurrentTargetActor;
	
	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 하이라이트 제어 헬퍼
	void UpdateHighlight(AActor* Target, bool bEnable);
	
	
};
