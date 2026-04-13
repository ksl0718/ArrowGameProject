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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	

	// 입력 시 실행될 로직
	void Input_DecoySkillA(const FInputActionValue& Value);
	
	// 은신 상태 복제 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsStealthed)
	bool bIsStealthed = false;

	UFUNCTION()
	void OnRep_IsStealthed();

	// 서버 스킬 실행
	UFUNCTION(Server, Reliable)
	void Server_UseDecoySkill(FVector SpawnLoc, FRotator SpawnRot);

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
