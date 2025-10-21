#include "UserCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "ArrowProjectile.h"

AUserCharacter::AUserCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 카메라 붐
    USpringArmComponent* CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 250.f;
    CameraBoom->bUsePawnControlRotation = true;
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

void AUserCharacter::StartCharging()
{
    if (!bIsAiming) return;

    bIsCharging = true;
    bIsTired = false;
    ChargeTime = 0.f;

    // TODO: 활 당기는 애니메이션 or 사운드 재생
    UE_LOG(LogTemp, Log, TEXT("Charging started."));
}

void AUserCharacter::ReleaseArrow()
{
    if (!bIsCharging || !bIsAiming) return;

    bIsCharging = false;
    bIsTired = false;

    float ChargeRatio = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);
    //속도 보간
    float ArrowSpeed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, ChargeRatio);

    // 라인트레이스
    FVector CameraLoc;
    FRotator CameraRot;
    GetController()->GetPlayerViewPoint(CameraLoc, CameraRot);

    FVector TraceStart = CameraLoc;
    FVector TraceEnd = TraceStart + (CameraRot.Vector() * 10000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
    // 맞은 곳이 있으면 거기로, 없으면 카메라 전방
    FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;

    //활 화살 스폰 포인트 -> 타겟 포인트를 바라보는 회전값 계산
    FVector SpawnLoc = ProjectileSpawnPoint->GetComponentLocation();
    FRotator SpawnRot = (TargetPoint - SpawnLoc).Rotation();

    //발사
    FireArrow(SpawnRot, ArrowSpeed);

    UE_LOG(LogTemp, Log, TEXT("Released arrow! Charge: %.2f (Speed %.0f)"), ChargeRatio, ArrowSpeed);

    ChargeTime = 0.f;
}

void AUserCharacter::StartAiming()
{
    bIsAiming = true;
    GetCharacterMovement()->MaxWalkSpeed = 200.f;
}

void AUserCharacter::StopAiming()
{
    bIsAiming = false;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void AUserCharacter::HandleDeath()
{
    Super::HandleDeath();

}

void AUserCharacter::OnDeath()
{
    DisableInput(Cast<APlayerController>(GetController()));
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]() { Destroy(); }, 3.0f, false);
}
void AUserCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 차징 중이면 ChargeTime 증가
    if (bIsCharging)
    {
        ChargeTime += DeltaTime;

        //힘든 상태 (한 번만 반응하도록 플래그)
        if (ChargeTime >= TiredThreshold && !bIsTired)
        {
            bIsTired = true;
            UE_LOG(LogTemp, Warning, TEXT("Archer is struggling..."));

            // 여기에 연출 추가 가능:
            // - 카메라 흔들림
            // - 숨소리/피로 사운드
            // - 애니메이션 블렌드 등
        }
        //제한 시간 넘으면 자동 발사
        if (ChargeTime >= AutoReleaseTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Charge timeout - auto release!"));
            ReleaseArrow(); // 강제 발사
            StopAiming();
            return; // ReleaseArrow() 내부에서 bIsCharging false로 변경됨
        }
    }

    float TargetFOV = bIsAiming ? AimFOV : NormalFOV;
    float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed);
    FollowCamera->SetFieldOfView(NewFOV);
}
