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
class ACharacter;

/**
 * 저주 중 AI 행동 모드 (Phase 1+에서 ApplyCurseControl 시 결정).
 * - FleeFromDokkaebi: 도깨비 반대 도망 (현재 구현).
 * - AttackAlly: 반경 내 궁수 + 시야(B: LineTrace 히트 액터 == 타겟) 시 아군 쪽 접근·자동 공격.
 */
UENUM(BlueprintType)
enum class ECurseBehaviorMode : uint8
{
	FleeFromDokkaebi UMETA(DisplayName = "Flee From Dokkaebi"),
	AttackAlly       UMETA(DisplayName = "Attack Ally"),
};

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

	/** 플레이어 수동 입력(이동·조준·발사 등)만 막음. 저주 AI(Tick/Brain)는 이 체크를 쓰지 않음. */
	UFUNCTION(BlueprintPure, Category = "Curse")
	bool IsPlayerManualInputBlockedByCurse() const { return bIsCursedControl; }

	/** @deprecated 이름 명확화 — IsPlayerManualInputBlockedByCurse 와 동일 */
	UFUNCTION(BlueprintPure, Category = "Curse", meta = (DeprecatedFunction, DeprecationMessage = "Use IsPlayerManualInputBlockedByCurse"))
	bool IsInputBlockedByCurse() const { return IsPlayerManualInputBlockedByCurse(); }

	UFUNCTION(BlueprintPure, Category = "Curse")
	ECurseBehaviorMode GetCursedBehaviorMode() const { return CursedBehaviorMode; }
	
	void UpdateCurseLocalPostProcessVignette();
#pragma endregion

protected:
#pragma region Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	UPROPERTY(EditAnywhere, Category = "Camera|Input", meta = (ClampMin = "0.01", ClampMax = "2.0"))
	float MouseSensitivity = 0.44f;

	UPROPERTY(EditAnywhere, Category = "Camera|Input", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float LookSmoothingSpeed = 15.f;

	FVector2D RawLookInput;
	FVector2D SmoothedLookInput;
#pragma endregion

#pragma region Curse_Internal
	void CursedBrainTick();

	bool IsCursedAndAlive() const { return bIsCursedControl && !bIsDead; }
	bool IsAttackAllyCurseMode() const { return CursedBehaviorMode == ECurseBehaviorMode::AttackAlly; }
	FVector GetCurseEyeWorldLocation(const AActor* Actor) const;

	/** 아군 궁수 + 반경 + LOS (TryResolve / 재판정 공용) */
	bool IsCurseAllyWithinAttackRange(const ACharacter* Candidate) const;

	/** 서버: TryResolve 결과로 모드·타겟·조준 갱신 */
	void ResolveAndApplyCurseBehaviorMode(bool bLogResolution);

	/** 서버: 저주 중 타겟 유효/시야 재검사 후 Flee↔AttackAlly 전환 */
	void ReevaluateCurseBehaviorOnAuthority();

	bool IsCurrentCurseAttackTargetValid() const;

	void DrawCurseDebugVisuals() const;

	/** 저주 AI 이동 입력 (소유 클라 / 리슨 서버 본인 궁수) */
	void ApplyCursedAutomatedMovementInput();

	/** AttackAlly + 유효 타겟일 때만. 소유 클라 ControlRotation → 타겟(에임 오프셋용) */
	void ApplyCursedAttackAllyLookFocus(float DeltaTime);

	bool ShouldFocusCurseAttackTarget() const;

	/** 서버: 모드에 따라 SetAiming / StopAiming */
	void ApplyCurseAimingForBehaviorMode();

	/** 소유 클라: AttackAlly 조준 UI·이동 속도 (SetAiming 복제 후) */
	void ApplyCurseAttackAimingLocalVisuals();

	/** 서버: AttackAlly 자동 발사 (간격·재장전·탄약 체크) */
	void TryCurseAutoFire();

	/** Goal 기준 XY 이동 방향. bMoveAwayFromGoal=true면 Goal에서 멀어지는 방향 */
	bool TryGetCursedHorizontalMoveDirection2D(
		const FVector& GoalWorldLocation,
		bool bMoveAwayFromGoal,
		FVector& OutDir) const;

	/** 모드별 Goal 선택 후 방향. AttackAlly 타겟 무효/사망 시 도망 폴백 */
	bool TryGetCursedMovementWorldDirection2D(FVector& OutDir) const;

	/** 서버 전용: 반경 내 궁수(!IsDokkaebi) 후보 + LineTrace B(히트 액터 == 후보) → 가장 가까운 1명 */
	bool TryResolveCurseAllyTarget(ACharacter*& OutTarget) const;

	bool IsCurseAllyArcher(const ACharacter* Candidate) const;
	bool HasClearCurseLineOfSightTo(const ACharacter* Candidate) const;

	UFUNCTION()
	void OnRep_IsCursedControl();

	UPROPERTY(ReplicatedUsing = OnRep_IsCursedControl)
	bool bIsCursedControl = false;

	/** ApplyCurseControl(서버)에서 설정. Phase 1 전까지 항상 FleeFromDokkaebi */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Curse|Behavior")
	ECurseBehaviorMode CursedBehaviorMode = ECurseBehaviorMode::FleeFromDokkaebi;

	/** AttackAlly 일 때 공격 대상 궁수. Phase 1+ */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Curse|Behavior")
	TObjectPtr<ACharacter> CursedAttackTarget = nullptr;

	/** 도망 방향 계산용 — 소유 클라에만 복제 (클라가 ServerMove에 도망 입력을 실어 보냄) */
	UPROPERTY(Replicated)
	ADokkaebiCharacter* CursedDokkaebi = nullptr;

	FTimerHandle CurseEndTimerHandle;

	/** 서버 저주 도망 시 AddMovementInput 스케일 (매 틱 적용, 1≈평소 걷기 가속에 가깝게) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|Flee")
	float CursedFleeInputScale = 1.f;

	/** AttackAlly: 이 반경(cm) 안 궁수만 후보. LineTrace B 통과 필요 */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	float CursedAllySearchRadius = 2000.f;

	/** AttackAlly: 타겟 접근 AddMovementInput 스케일 */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	float CursedApproachInputScale = 1.f;

	/** AttackAlly: 시야 판정 LineTrace 채널 (기본 Visibility = 벽 차단) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	TEnumAsByte<ECollisionChannel> CursedAllyLineTraceChannel = ECC_Visibility;

	/** AttackAlly: LineTrace 시작/끝 Z 오프셋 (캡슐 중심 기준, 눈 높이 근사) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	float CursedAllyTraceEyeHeight = 60.f;

	/** 서버: 아군 판정 실패 이유 로그 (Visibility가 Pawn에 안 맞을 때 디버그) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	bool bLogCurseAllyResolve = false;

	/** Phase 3: 타겟 LookAt 보간 속도 (0이면 즉시 SetControlRotation) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	float CursedAllyLookInterpSpeed = 12.f;

	/** Phase 5: 자동 발사 최소 간격(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly")
	float CurseAutoFireInterval = 1.2f;

	/** Phase 5: 자동 발사 차지율 (0~1, Bow MaxChargeTime 대비) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|AttackAlly", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurseAutoFireChargePercent = 0.7f;

	/** CursedBrainTick 서버 디버그 로그 스로틀용 (초 단위 GetTimeSeconds 비교) */
	float CurseMoveDebugLastLogTime = -1.0e9f;

	/** Phase 5: 마지막 저주 자동 발사 시각 (서버 GetTimeSeconds) */
	float LastCurseAutoFireTime = -1.0e9f;

	/** 저주 중 주기적으로 Flee↔AttackAlly 재판정 (서버) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|Behavior")
	bool bReevaluateCurseBehaviorWhileActive = true;

	UPROPERTY(EditDefaultsOnly, Category = "Curse|Behavior", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "2.0"))
	float CurseBehaviorReevaluateInterval = 0.5f;

	float LastCurseBehaviorReevaluateTime = -1.0e9f;

	/** 저주 중 이동·LOS·타겟 디버그 라인 (PIE) */
	UPROPERTY(EditDefaultsOnly, Category = "Curse|Debug")
	bool bDrawCurseDebug = false;

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

	bool bArcherVisionActive = false;
	void ApplyDokkaebiVision(bool bEnable);

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
