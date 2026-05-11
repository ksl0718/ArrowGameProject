// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcherCharacterBase.h"
#include "SkillCooldownProvider.h"
#include "InputActionValue.h"
#include "../Weapon/Bow.h"
#include "UserArcherCharacter.generated.h"

class UTexture2D;
class UBowReticleWidget;
class UCameraShakeBase;
class ADokkaebiCharacter;

/**
 * Player-controlled archer (Enhanced Input, reticle, roll skill, curse).
 */
UCLASS()
class ARROWGAME_API AUserArcherCharacter : public AArcherCharacterBase, public ISkillCooldownProvider
{
	GENERATED_BODY()

public:
#pragma region Constructor
	AUserArcherCharacter();
#pragma endregion

#pragma region SkillCooldownProvider
	virtual int32 GetSkillSlotCount() const override;
	virtual bool GetSkillHudMetaByIndex(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const override;
	virtual bool GetSkillCooldownByIndex(int32 SlotIndex, float& OutRemaining, float& OutDuration) const override;
#pragma endregion

#pragma region Stats
	bool IsDead() const { return bIsDead; }

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bCanMove = true;
#pragma endregion

#pragma region RollSkill
	UFUNCTION(BlueprintPure, Category = "User|Skill")
	float GetRollCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "User|Skill")
	float GetRollCooldownDuration() const { return RollCooldownDuration; }
#pragma endregion

#pragma region Equipment
	void EquipNewBow(TSubclassOf<ABow> NewBowClass);
#pragma endregion

#pragma region Curse
	UFUNCTION(BlueprintCallable, Category = "Curse")
	void ApplyCurseControl(float Duration, ADokkaebiCharacter* CurseSource);

	UFUNCTION(BlueprintCallable, Category = "Curse")
	void EndCurseControl();

	UFUNCTION(BlueprintPure, Category = "Curse")
	bool IsCursedControl() const { return bIsCursedControl; }

	UFUNCTION(BlueprintPure, Category = "Curse")
	bool IsInputBlockedByCurse() const { return bIsCursedControl; }
#pragma endregion

protected:
#pragma region Lifecycle
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
#pragma endregion

#pragma region Input_Assets
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* CrouchAction;
#pragma endregion

#pragma region Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float NormalFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float AimFOV = 65.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float AimInterpSpeed = 15.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Shake")
	TSubclassOf<UCameraShakeBase> TiredCameraShakeClass;

	bool bTiredShakeActive = false;
#pragma endregion

#pragma region Curse_Internal
	void CursedBrainTick();

	/** 저주 도망 방향(월드 XY). 도깨비 복제 위치 기준, 실패 시 false */
	bool TryGetCursedFleeWorldDirection2D(FVector& OutDir) const;

	UFUNCTION()
	void OnRep_IsCursedControl();

	UPROPERTY(ReplicatedUsing = OnRep_IsCursedControl)
	bool bIsCursedControl = false;

	/** 도망 방향 계산용 — 소유 클라에만 복제 (클라가 ServerMove에 도망 입력을 실어 보냄) */
	UPROPERTY(Replicated)
	ADokkaebiCharacter* CursedDokkaebi = nullptr;

	FTimerHandle CurseEndTimerHandle;

	/** 서버 저주 도망 시 AddMovementInput 스케일 (매 틱 적용, 1≈평소 걷기 가속에 가깝게) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|Flee")
	float CursedFleeInputScale = 1.f;

	/** 도깨비와의 거리가 이 값보다 크면 도망 이동 중지 (0이면 제한 없음) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|Flee")
	float CursedFleeMaxDistance = 0.f;

	/** CursedBrainTick 서버 디버그 로그 스로틀용 (초 단위 GetTimeSeconds 비교) */
	float CurseMoveDebugLastLogTime = -1.0e9f;

#pragma endregion

#pragma region Input_Handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnJumpInput();
	void StartAiming();
	void StopAiming();
	void StartCharging();
	void ReleaseArrow();

	void OnCrouchStarted(const FInputActionValue& Value);
	void OnCrouchEnded(const FInputActionValue& Value);

	void Input_CycleArrow(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);

	void OnWalkSlowStarted(const FInputActionValue& Value);
	void OnWalkSlowEnded(const FInputActionValue& Value);


	UFUNCTION(Server, Reliable)
	void ServerSetMaxWalkSpeed(float NewSpeed);

	void Roll();

	UFUNCTION(Server, Reliable)
	void ServerPlayRoll();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayRoll();

	UFUNCTION(BlueprintCallable)
	void OnRollEnd(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

#pragma region RollSkill_Data
	UPROPERTY(EditAnywhere, Category = "User|Skill")
	float RollCooldownDuration = 4.0f;

	UPROPERTY(EditAnywhere, Category = "User|Skill|UI")
	UTexture2D* RollSkillIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "User|Skill|UI")
	FText RollSkillKeyText = FText::FromString(TEXT("LShift"));

	float NextRollAvailableTime = 0.0f;
#pragma endregion

#pragma region UI_Reticle
	UPROPERTY(EditAnywhere, Category = "UI|Reticle")
	TSubclassOf<UBowReticleWidget> ReticleWidgetClass;

	UPROPERTY()
	UBowReticleWidget* ReticleWidget = nullptr;

	void ShowReticle();
	void HideReticle();
	void StopTiredShake();
#pragma endregion


#pragma region Interaction

	FTimerHandle RollSafetyTimerHandle;
	void OnRollSafetyTimeout();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class USphereComponent* InteractionSphere;

	UPROPERTY()
	TArray<AActor*> OverlappingActors;

	UPROPERTY()
	AActor* CurrentTargetActor;

	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateHighlight(AActor* Target, bool bEnable);

	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* HitActor);
#pragma endregion
};
