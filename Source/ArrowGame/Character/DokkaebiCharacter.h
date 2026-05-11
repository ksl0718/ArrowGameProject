#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "SkillCooldownProvider.h"
#include "InputActionValue.h"
#include "ArrowGame/UI/SpiritSightMarkerWidget.h"
#include "DokkaebiCharacter.generated.h"

#pragma region ForwardDeclarations
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UTexture2D;
class ADokkaebiDecoy;
class ADokkaebiCurseProjectile;
class UUserWidget;

#pragma endregion

#pragma region SkillStructs

USTRUCT(BlueprintType)
struct FSkillSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	float Cooldown = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	float InputLockDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill|UI")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill|UI")
	FText KeyText;
};

USTRUCT()
struct FSkillRuntimeState
{
	GENERATED_BODY()
	
	UPROPERTY()
	float NextAvailableTime = 0.f;
	
	UPROPERTY()
	bool bInputLocked = false;
};

/** SkillSpecs / TryCommit 인덱스. 실제 매핑: 0 미끼, 1 저주, 2 투시(혼 시야). */
UENUM(BlueprintType)
enum class EDokkaebiSkillIndex : uint8
{
	Decoy = 0,
	SkillB = 1,
	SkillC = 2,
	SkillD = 3
};


#pragma endregion

/** 플레이어 도깨비: 미끼·은신, 저주 탄, 투시(적 스크린 마커). 스킬 쿨다운은 SkillSpecs·SkillStates로 관리. */
UCLASS()
class ARROWGAME_API ADokkaebiCharacter : public ACharacterBase, public ISkillCooldownProvider
{
	GENERATED_BODY()

public:
	ADokkaebiCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual int32 GetSkillSlotCount() const override;
	virtual bool GetSkillHudMetaByIndex(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const override;
	virtual bool GetSkillCooldownByIndex(int32 SlotIndex, float& OutRemaining, float& OutDuration) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Skill_API // HUD 쿨다운용; NextAvailableTime은 서버 월드 시간 기준
public:

	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetSkillCooldownRemainingByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetSkillCooldownDurationByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	bool IsSkillCoolingDownByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	bool CanUseSkillOnAuthority(int32 SkillIndex) const;
	bool TryCommitSkillUseOnAuthority(int32 SkillIndex);
	void UnlockSkillInput(int32 SkillIndex);
	
	UPROPERTY(Replicated)
	TArray<FSkillRuntimeState> SkillStates;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	TArray<FSkillSpec> SkillSpecs;
	
#pragma endregion

#pragma region Decoy_Skill // 미끼 스폰 + 전 클라에 보이는 은신
protected:
	void Input_DecoySkillA(const FInputActionValue& Value);

	/** 은신 여부 — 모든 관련 클라에 복제 */
	UPROPERTY(ReplicatedUsing = OnRep_IsStealthed)
	bool bIsStealthed = false;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float StealthDuration = 3.0f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnForwardOffset = 80.f;

	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnUpOffset = 40.f;

	UFUNCTION()
	void OnRep_IsStealthed();

	UFUNCTION(Server, Reliable)
	void Server_UseDecoySkill(FVector SpawnLoc, FRotator SpawnRot);

	void ExecuteDecoySkillOnAuthority(FVector SpawnLoc, FRotator SpawnRot);
	void EndStealthOnAuthority();
	bool CanUseDecoySkillOnAuthority() const;

	FTimerHandle StealthEndTimerHandle;
	FTimerHandle SkillInputUnlockTimerHandle;
#pragma endregion

#pragma region Curse_Skill // 투사체 스폰, SkillSpecs[1] 쿨다운
protected:
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	TSubclassOf<ADokkaebiCurseProjectile> CurseProjectileClass;
	
	UFUNCTION(Server, Reliable)
	void Server_FireCurseProjectile(FVector SpawnLoc, FRotator SpawnRot);
	
	void FireCurseProjectile();
	
#pragma endregion
	
#pragma region Wallhack_Skill // 투시: 서버에서 종료 시각만 복제, 마커는 소유 클라에서만 갱신
	
protected:
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight")
	float SpiritSightDuration = 9.f;

	/** 카메라~적까지 이 거리(cm) 이하면 마커 스케일 = SpiritSightScaleAtNear */
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight", meta = (ClampMin = 1.f))
	float SpiritSightScaleNearCm = 600.f;

	/** 이 거리(cm) 이상이면 마커 스케일 = SpiritSightScaleAtFar (그 사이는 선형) */
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight", meta = (ClampMin = 1.f))
	float SpiritSightScaleFarCm = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight", meta = (ClampMin = 0.05f, ClampMax = 3.f))
	float SpiritSightScaleAtNear = 1.05f;

	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight", meta = (ClampMin = 0.05f, ClampMax = 3.f))
	float SpiritSightScaleAtFar = 0.28f;
	
	/** 서버 시간 기준 투시 종료 시각. 비활성은 Now >= 이 값. OwnerOnly 복제. */
	UPROPERTY(ReplicatedUsing = OnRep_SpiritSightEnd)
	float SpiritSightEndServerTime = 0.f;
	
	UFUNCTION()
	void OnRep_SpiritSightEnd();
	
	UFUNCTION(Server, Reliable)
	void Server_UseSpiritSight();

	/** 서버 전용: 쿨다운 커밋 + 종료 시각 설정 */
	void ExecuteSpiritSightOnAuthority();
	
	void Input_SpiritSight(const FInputActionValue& Value);
	
	/** 적 스크린 좌표 수집 → 위젯에 전달 */
	void UpdateSpiritSightMarkers(float DeltaTime);

	/** 빙의 후 PC 준비될 때 마커 위젯 1회 생성 */
	void EnsureSpiritSightMarkerWidget();
	
	bool IsSpiritSightActive_ServerTime() const;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SpiritSightAction;
	
	/** 기본 USpiritSightMarkerWidget(C++)만 지정해도 됨. BP는 선택(연출 오버라이드용) */
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight")
	TSubclassOf<USpiritSightMarkerWidget> SpiritSightMarkerWidgetClass;

	/** 마커 1개용 WBP. 비우면 SpiritSightMarkerWidget Class Defaults의 MarkerEntryWidgetClass 사용 */
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill|SpiritSight")
	TSubclassOf<UUserWidget> SpiritSightMarkerEntryClass;

	UPROPERTY()
	TObjectPtr<USpiritSightMarkerWidget> SpiritSightMarkerWidget;
	
#pragma endregion
	
#pragma region Skill_Config
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill")
	TSubclassOf<ADokkaebiDecoy> DecoyClass;
#pragma endregion

#pragma region Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DecoySkillAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CurseSkillAction;
	
#pragma endregion

#pragma region Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;
#pragma endregion

#pragma region Movement
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float NormalWalkSpeed = 400.f;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
#pragma endregion
};
