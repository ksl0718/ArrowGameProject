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

void ADokkaebiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADokkaebiCharacter, bIsStealthed);
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
	if (!HasAuthority() || bIsDead || bIsStealthed)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Dokkaebi: ExecuteDecoySkillOnAuthority (listen server / dedicated)"));

	bIsStealthed = true;
	OnRep_IsStealthed();

	if (DecoyClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		GetWorld()->SpawnActor<ADokkaebiDecoy>(DecoyClass, SpawnLoc, SpawnRot, Params);
	}

	GetWorldTimerManager().ClearTimer(StealthEndTimerHandle);
	GetWorldTimerManager().SetTimer(
		StealthEndTimerHandle,
		[this]()
		{
			bIsStealthed = false;
			OnRep_IsStealthed();
		},
		3.0f,
		false);
}

void ADokkaebiCharacter::Input_DecoySkillA(const FInputActionValue& Value)
{
	const FVector SpawnLoc = GetActorLocation();
	const FRotator SpawnRot = GetControlRotation();

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
		GetMesh()->SetScalarParameterValueOnMaterials(TEXT("Opacity"),bIsStealthed ? 0.3f : 1.0f);
	}
	else
	{
		GetMesh()->SetHiddenInGame(bIsStealthed);
		
	}
}
