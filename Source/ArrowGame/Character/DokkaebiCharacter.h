#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputActionValue.h"
#include "DokkaebiCharacter.generated.h"

#pragma region ForwardDeclarations
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UTexture2D;
class ADokkaebiDecoy;
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

UENUM(BlueprintType)
enum class EDokkaebiSkillIndex : uint8
{
	Decoy = 0,
	SkillB = 1,
	SkillC = 2,
	SkillD = 3
};


#pragma endregion



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

#pragma region Skill_API
public:

	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetSkillCooldownRemainingByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	float GetSkillCooldownDurationByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	UFUNCTION(BlueprintPure, Category="Dokkaebi|Skill")
	bool IsSkillCoolingDownByIndex(EDokkaebiSkillIndex SkillIndex) const;
	
	UPROPERTY(Replicated)
	TArray<FSkillRuntimeState> SkillStates;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	TArray<FSkillSpec> SkillSpecs;
	
#pragma endregion

#pragma region Decoy_Skill
protected:
	// Input entry point for decoy skill.
	void Input_DecoySkillA(const FInputActionValue& Value);

	// Stealth state replicated to all relevant clients.
	UPROPERTY(ReplicatedUsing = OnRep_IsStealthed)
	bool bIsStealthed = false;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float StealthDuration = 3.0f;
	
	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnForwardOffset = 80.f;

	UPROPERTY(EditAnywhere, Category="Dokkaebi|Skill")
	float DecoySpawnUpOffset = 10.f;

	UFUNCTION()
	void OnRep_IsStealthed();

	// Client -> server skill request RPC.
	UFUNCTION(Server, Reliable)
	void Server_UseDecoySkill(FVector SpawnLoc, FRotator SpawnRot);

	// Runs actual skill logic on authority only.
	void ExecuteDecoySkillOnAuthority(FVector SpawnLoc, FRotator SpawnRot);

	// Shared end-stealth processing.
	void EndStealthOnAuthority();

	// Cooldown/lock/state validation.
	bool CanUseDecoySkillOnAuthority() const;

	FTimerHandle StealthEndTimerHandle;
	FTimerHandle SkillInputUnlockTimerHandle;
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
