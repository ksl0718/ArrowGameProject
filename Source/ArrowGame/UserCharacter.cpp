#include "UserCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "ArrowProjectile.h"
#include "Bow.h"

AUserCharacter::AUserCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 카메라 붐
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 250.f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->SocketOffset = FVector(0.f, 60.f, 60.f);
    CameraBoom->TargetOffset = FVector(0.f, 0.f, 30.f);

    // 팔로우 카메라
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AUserCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 등록
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

        //에임 모드 -  (RMB)
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AUserCharacter::StartAiming);
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AUserCharacter::StopAiming);

        // 차징 & 발사 (LMB)
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Started, this, &AUserCharacter::StartCharging);
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Completed, this, &AUserCharacter::ReleaseArrow);
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

    // --- 조준 시 몸이 카메라 따라 돌도록 설정 ---
    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = true;

    if (EquippedWeapon)
    {
        EquippedWeapon->StartAim();
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

    // TODO: 활 당기는 애니메이션 or 사운드 재생
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
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    /// --- 조준 종료 시 다시 이동 방향 바라보게 ---
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;

    if (EquippedWeapon)
    {
        EquippedWeapon->StopAim();
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
            Bow->StartAim();  // 화살 리필
        }
    }
    float TargetFOV = bAiming ? AimFOV : NormalFOV;
    float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed);
    FollowCamera->SetFieldOfView(NewFOV);
}
