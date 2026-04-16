#include "DokkaebiCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "../Character/DokkaebiDecoy.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

void ADokkaebiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADokkaebiCharacter, bIsStealthed);
	DOREPLIFETIME_CONDITION(ADokkaebiCharacter, SkillStates, COND_OwnerOnly);
}

ADokkaebiCharacter::ADokkaebiCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 60.f);
	CameraBoom->TargetOffset = FVector(0.f, 0.f, 30.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}
}

void ADokkaebiCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NormalWalkSpeed;
	}
	SkillStates.SetNum(SkillSpecs.Num());
}


void ADokkaebiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DefaultMappingContext)
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (PC->IsLocalPlayerController())
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
						ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
					{
						Subsystem->RemoveMappingContext(DefaultMappingContext);
					}
				}
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ADokkaebiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADokkaebiCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADokkaebiCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (DecoySkillAction)
		{
			EnhancedInput->BindAction(DecoySkillAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::Input_DecoySkillA);
		}
	}

	// BeginPlay 때는 아직 Possess 전이라 Controller가 없을 수 있음(호스트에서 특히).
	// 빙의 직후 이 함수가 호출될 때만 로컬 서브시스템에 IMC를 올린다.
	if (DefaultMappingContext)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->IsLocalPlayerController())
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
						ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
					{
						Subsystem->AddMappingContext(DefaultMappingContext, 0);
					}
				}
			}
		}
	}
}

void ADokkaebiCharacter::Move(const FInputActionValue& Value)
{
	if (!bCanMove || bIsDead)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir, MovementVector.X);
	}
}

void ADokkaebiCharacter::Look(const FInputActionValue& Value)
{
	if (bIsDead)
	{
		return;
	}

	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void ADokkaebiCharacter::Server_UseDecoySkill_Implementation(FVector SpawnLoc, FRotator SpawnRot)
{
	ExecuteDecoySkillOnAuthority(SpawnLoc, SpawnRot);
}

void ADokkaebiCharacter::ExecuteDecoySkillOnAuthority(FVector SpawnLoc, FRotator SpawnRot)
{
	if (!CanUseDecoySkillOnAuthority())
	{
		return;
	}
	
	constexpr int32 DecoyIndex = 0;
	
	if (!SkillStates.IsValidIndex(DecoyIndex) || !SkillSpecs.IsValidIndex(DecoyIndex)) return;
	
	FSkillRuntimeState& State = SkillStates[DecoyIndex];
	const FSkillSpec& Spec = SkillSpecs[DecoyIndex];
	
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	State.bInputLocked = true;
	State.NextAvailableTime = NowServer + Spec.Cooldown;

	GetWorldTimerManager().ClearTimer(SkillInputUnlockTimerHandle);
	GetWorldTimerManager().SetTimer(
		SkillInputUnlockTimerHandle,
		[this, DecoyIndex]()
		{
			if (SkillStates.IsValidIndex(DecoyIndex))
			{
				SkillStates[DecoyIndex].bInputLocked = false;
			}
		},
		Spec.InputLockDuration,
		false
	);
	
	bIsStealthed = true;
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Stealth ON set. Auth=%d Local=%d NetMode=%d Time=%.2f"),
    HasAuthority() ? 1 : 0,
    IsLocallyControlled() ? 1 : 0,
    (int32)GetNetMode(),
    GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f);
	OnRep_IsStealthed();
	
	const FVector Forward = SpawnRot.Vector();
	const FVector SafeSpawnLoc = SpawnLoc + Forward * DecoySpawnForwardOffset + FVector(0,0,DecoySpawnUpOffset);
	
	if (!DecoyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DecoyClass is null"));
	}else
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ADokkaebiDecoy* Decoy = GetWorld()->SpawnActor<ADokkaebiDecoy>(DecoyClass, SafeSpawnLoc, SpawnRot, Params);
		if (!Decoy)
		{
			UE_LOG(LogTemp, Error, TEXT("Decoy spawn failed at %s"), *SafeSpawnLoc.ToString());
		}
	}
	GetWorldTimerManager().ClearTimer(StealthEndTimerHandle);
	GetWorldTimerManager().SetTimer(
		StealthEndTimerHandle,
		this,
		&ADokkaebiCharacter::EndStealthOnAuthority,
		StealthDuration,
		false
	);
}

bool ADokkaebiCharacter::CanUseDecoySkillOnAuthority() const
{
	constexpr int32 DecoyIndex = 0;
	
	if (!HasAuthority()) return false;
	if (bIsDead) return false;
	if (bIsStealthed) return false;
	if (!SkillStates.IsValidIndex(DecoyIndex)) return false;
	
	const float Now = GetWorld()->GetTimeSeconds();
	if (SkillStates[DecoyIndex].bInputLocked) return false;
	if (Now < SkillStates[DecoyIndex].NextAvailableTime) return false;
	
	return true;
}

void ADokkaebiCharacter::EndStealthOnAuthority()
{
	if (!HasAuthority()) return;
	bIsStealthed = false;

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Stealth OFF set. Auth=%d Local=%d NetMode=%d Time=%.2f"),
        HasAuthority() ? 1 : 0,
        IsLocallyControlled() ? 1 : 0,
        (int32)GetNetMode(),
        GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f);

	OnRep_IsStealthed();
}

void ADokkaebiCharacter::Input_DecoySkillA(const FInputActionValue& Value)
{
	const FVector SpawnLoc = GetActorLocation();
	const FRotator ControlRot = GetControlRotation();
	const FRotator SpawnRot(0.f, ControlRot.Yaw, 0.f);
	
	// 리스닝 서버 호스트는 이미 Authority — Server RPC만 호출하면 구현이 안 도는 경우가 있어 직접 실행.
	if (HasAuthority())
	{
		ExecuteDecoySkillOnAuthority(SpawnLoc, SpawnRot);
	}
	else
	{
		Server_UseDecoySkill(SpawnLoc, SpawnRot);
	}

	UE_LOG(LogTemp, Log, TEXT("Dokkaebi: Skill Input Pressed (Authority=%d)"), HasAuthority() ? 1 : 0);
}

void ADokkaebiCharacter::OnRep_IsStealthed()
{
	

	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Stealth OnRep: bIsStealthed=%d, Local=%d"),
		bIsStealthed ? 1 : 0,
		IsLocallyControlled() ? 1 : 0);

		GetMesh()->SetScalarParameterValueOnMaterials(TEXT("Opacity"),bIsStealthed ? 0.3f : 1.0f);
	}
	else
	{
		GetMesh()->SetHiddenInGame(bIsStealthed);
		
	}
}

float ADokkaebiCharacter::GetSkillCooldownRemainingByIndex(EDokkaebiSkillIndex SkillIndex) const
{
	if (!GetWorld()) return 0.f;

	const int32 Index = static_cast<int32>(SkillIndex);
	if (!SkillStates.IsValidIndex(Index)) return 0.f;

	const AGameStateBase* GS = GetWorld()->GetGameState();
	const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	return FMath::Max(0.f, SkillStates[Index].NextAvailableTime - NowServer);
}
float ADokkaebiCharacter::GetSkillCooldownDurationByIndex(EDokkaebiSkillIndex SkillIndex) const
{
	const int32 Index = static_cast<int32>(SkillIndex);
	if (!SkillSpecs.IsValidIndex(Index)) return 0.01f;
	
	return FMath::Max(0.01f, SkillSpecs[Index].Cooldown);
}

bool ADokkaebiCharacter::IsSkillCoolingDownByIndex(EDokkaebiSkillIndex SkillIndex) const
{
	return GetSkillCooldownRemainingByIndex(SkillIndex) > 0.f;
}