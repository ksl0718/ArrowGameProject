#include "UserCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "../Weapon/ArrowProjectile.h"
#include "../Weapon/Bow.h"
#include "GameFramework/CharacterMovementComponent.h"

AUserCharacter::AUserCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ī�޶� ��
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 250.f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->SocketOffset = FVector(0.f, 60.f, 60.f);
    CameraBoom->TargetOffset = FVector(0.f, 0.f, 30.f);

    // �ȷο� ī�޶�
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AUserCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

    // Enhanced Input ���
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AUserCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUserCharacter::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUserCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);

        //���� ��� -  (RMB)
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AUserCharacter::StartAiming);
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AUserCharacter::StopAiming);

        // ��¡ & �߻� (LMB)
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Started, this, &AUserCharacter::StartCharging);
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Completed, this, &AUserCharacter::ReleaseArrow);

        //�ȱ�
        EnhancedInput->BindAction(WalkAction, ETriggerEvent::Started, this, &AUserCharacter::OnWalkSlowStarted);
        EnhancedInput->BindAction(WalkAction, ETriggerEvent::Completed, this, &AUserCharacter::OnWalkSlowEnded);
        
        //���̺�
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Started, this, &AUserCharacter::Roll);
    }
}

void AUserCharacter::Move(const FInputActionValue& Value)
{
    if (!bCanMove) return;

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

void AUserCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void AUserCharacter::StartAiming()
 {
     if (!bCanMove) return;
     GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
     // --- ���� �� ���� ī�޶� ���� ������ ���� ---
     SetAiming(true);
 
     if (EquippedWeapon)
     {
         ABow* Bow = Cast<ABow>(EquippedWeapon);
         if (Bow)
         {
             Bow->SetAiming(true);
         }
     }else
     {
         UE_LOG(LogTemp, Error, TEXT("not spawned Weapon"));
     }
 
 }

void AUserCharacter::StartCharging()
{

    if (!EquippedWeapon) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow || !Bow->IsAiming()) return;


    if (EquippedWeapon)
    {
        EquippedWeapon->StartDraw();
    }

    // TODO: Ȱ ���� �ִϸ��̼� or ���� ���
    UE_LOG(LogTemp, Log, TEXT("Charging started."));
}

void AUserCharacter::ReleaseArrow()
{
    if (!EquippedWeapon) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow) return;

    if (!Bow->IsCharging())
        return;

    Bow->EndDraw();
}



void AUserCharacter::StopAiming()
{
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    SetAiming(false);

    if (EquippedWeapon)
    {
        ABow* Bow = Cast<ABow>(EquippedWeapon);
        if (Bow)
        {
            Bow->SetAiming(false);
        }
    }

}

void AUserCharacter::HandleDeath()
{
    Super::HandleDeath();

}

void AUserCharacter::OnDeath()
{
	bIsDead = true;
    
    DisableInput(Cast<APlayerController>(GetController()));
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]() { Destroy(); }, 3.0f, false);
    
}
void AUserCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ABow* Bow = Cast<ABow>(EquippedWeapon);
    bool bAiming = false;

    if (Bow) {
        bAiming = Bow->IsAiming();
        if (Bow->IsAiming() && Bow->PreparedArrow == nullptr)
        {
            Bow->StartAim();  // ȭ�� ����
        }
    }
    float TargetFOV = bAiming ? AimFOV : NormalFOV;
    float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed);
    FollowCamera->SetFieldOfView(NewFOV);
}

void AUserCharacter::OnWalkSlowStarted(const FInputActionValue& Value)
{
    // ����Ʈ ������ ���� �� ������ �ȱ�
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AUserCharacter::OnWalkSlowEnded(const FInputActionValue& Value)
{
    // ����Ʈ ���� ���� �� ���� �ӵ�
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AUserCharacter::Roll()
{
    if (bIsRolling || bIsDead || !bCanMove) return;
    if (!RollMontage) return; 

    bIsRolling = true;
    bCanMove = false;

    PlayMontage(RollMontage, 1.f);
    
    ServerPlayRoll();
}

void AUserCharacter::ServerPlayRoll_Implementation()
{
    MulticastPlayRoll();
}

void AUserCharacter::MulticastPlayRoll_Implementation()
{
    if (!IsLocallyControlled()) 
    {
        // 몽타주 재생
        PlayAnimMontage(RollMontage);
        bIsRolling = true;

        // 여기도 마찬가지로 끝나면 상태를 풀어줘야 함 (남의 화면에서도 상태 관리가 필요하므로)
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AUserCharacter::OnRollEnd);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
        }
    }
}

void AUserCharacter::OnRollEnd(UAnimMontage* Montage, bool bInterrupted)
{
	bIsRolling = false;
	bCanMove = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}