#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputActionValue.h"
#include "DokkaebiCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class ADokkaebiDecoy;

UCLASS()
class ARROWGAME_API ADokkaebiCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	ADokkaebiCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
#pragma region Decoy
	
public:
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetDecoyCooldownRemaining() const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetDecoyCooldownDuration() const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	bool IsDecoyCoolingDown() const;
	
protected:
	// 입력 시 실행될 로직
	void Input_DecoySkillA(const FInputActionValue& Value);
	
	// 은신 상태 복제 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsStealthed)
	bool bIsStealthed = false;

	// 입력 연타 방지용 짧은 락 (서버 권한 상태)
	bool bDecoyInputLocked = false;

	// 서버 시간 기준 쿨다운 종료 시각
	float NextSkillAvailableTime = 0.f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float StealthDuration = 3.0f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float SkillCooldown = 8.0f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float InputLockDuration = 0.2f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnForwardOffset = 80.f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnUpOffset = 10.f;
	
	UFUNCTION()
	void OnRep_IsStealthed();

	// 서버 스킬 실행 (클라 → 서버). 리스닝 서버 호스트는 Input 쪽에서 Authority로 직접 실행.
	UFUNCTION(Server, Reliable)
	void Server_UseDecoySkill(FVector SpawnLoc, FRotator SpawnRot);

	/** 서버(또는 리스닝 서버 호스트)에서만 호출 — RPC와 동일한 본문 */
	void ExecuteDecoySkillOnAuthority(FVector SpawnLoc, FRotator SpawnRot);

	// 공통 종료 처리
	void EndStealthOnAuthority();
	// 쿨다운/락/상태 검사
	bool CanUseDecoySkillOnAuthority() const;
	
	FTimerHandle StealthEndTimerHandle;
	FTimerHandle SkillInputUnlockTimerHandle;
	
#pragma endregion
	
	UPROPERTY(EditAnywhere, Category = "Dokkaebi|Skill")
	TSubclassOf<ADokkaebiDecoy> DecoyClass;
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float NormalWalkSpeed = 400.f;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	
};
