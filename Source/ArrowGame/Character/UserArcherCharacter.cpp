#include "UserArcherCharacter.h"
#include "Animation/AnimInstance.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "../Weapon/ArrowProjectile.h"
#include "../Weapon/Bow.h"
#include "../Actor/ArrowItem.h"
#include "ArrowGame/Actor/BowItem.h"
#include "ArrowGame/UI/BowReticleWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h" 
#include "ArrowGame/Character/DokkaebiCharacter.h"
#include "ArrowGame/Core/ArrowGameState.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void AUserArcherCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AUserArcherCharacter, bIsCursedControl);
    DOREPLIFETIME(AUserArcherCharacter, CursedBehaviorMode);
    DOREPLIFETIME(AUserArcherCharacter, CursedAttackTarget);
    DOREPLIFETIME_CONDITION(AUserArcherCharacter, CursedDokkaebi, COND_OwnerOnly);
}

AUserArcherCharacter::AUserArcherCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    
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
    
    // 센서 컴포넌트 생성 및 설정
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetSphereRadius(150.0f); // 감지 범위
    //InteractionSphere->SetHiddenInGame(false);
    
    // 상호작용 채널에만 반응하도록 설정 (성능 최적화)
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    InteractionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
}

int32 AUserArcherCharacter::GetSkillSlotCount() const
{
    return 1;
}


bool AUserArcherCharacter::GetSkillHudMetaByIndex(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const
{
    OutIcon = nullptr;
    OutKeyText = FText::GetEmpty();
    
    if (SlotIndex != 0) return false;
    OutIcon = RollSkillIcon;
    OutKeyText = RollSkillKeyText;
    return true;
}
bool AUserArcherCharacter::GetSkillCooldownByIndex(int32 SlotIndex, float& OutRemaining, float& OutDuration) const
{
    OutRemaining = 0.f;
    OutDuration = 0.01f;
    
    if (SlotIndex != 0) return false;
    OutRemaining = GetRollCooldownRemaining();
    OutDuration = GetRollCooldownDuration();
    return true;
}

void AUserArcherCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

    // 이벤트 바인딩
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUserArcherCharacter::OnInteractionOverlapBegin);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUserArcherCharacter::OnInteractionOverlapEnd);
    
}

void AUserArcherCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void AUserArcherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUserArcherCharacter::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUserArcherCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AUserArcherCharacter::OnJumpInput);

        //���� ��� -  (RMB)
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AUserArcherCharacter::StartAiming);
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AUserArcherCharacter::StopAiming);
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &AUserArcherCharacter::StopAiming);

        // ��¡ & �߻� (LMB)
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Started, this, &AUserArcherCharacter::StartCharging);
        EnhancedInput->BindAction(ShootAction, ETriggerEvent::Completed, this, &AUserArcherCharacter::ReleaseArrow);

        //�ȱ�
        EnhancedInput->BindAction(WalkAction, ETriggerEvent::Started, this, &AUserArcherCharacter::OnWalkSlowStarted);
        EnhancedInput->BindAction(WalkAction, ETriggerEvent::Completed, this, &AUserArcherCharacter::OnWalkSlowEnded);
        
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AUserArcherCharacter::OnCrouchStarted);
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AUserArcherCharacter::OnCrouchEnded);
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &AUserArcherCharacter::OnCrouchEnded);
        
        //���̺�
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Started, this, &AUserArcherCharacter::Roll);
        
        //화살변경
        EnhancedInput->BindAction(CycleArrowAction, ETriggerEvent::Triggered, this, &AUserArcherCharacter::Input_CycleArrow);
        //화살 pickup
        EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AUserArcherCharacter::Input_Interact);
    }

    // 빙의 직후에만 호출됨 — BeginPlay보다 늦게 Controller/LocalPlayer가 준비된 시점 (도깨비와 동일)
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
                        UpdateCurseLocalPostProcessVignette();
                    }
                }
            }
        }
    }
}

void AUserArcherCharacter::Move(const FInputActionValue& Value)
{
    if (IsPlayerManualInputBlockedByCurse()) return;
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

void AUserArcherCharacter::Look(const FInputActionValue& Value)
{
    // 저주 중에도 시야(마우스)는 돌릴 수 있게 둔다. 이동만 IsPlayerManualInputBlockedByCurse 로 막는다.
    // Tick에서 스무딩 후 적용하므로 여기서는 누적만 한다.
    RawLookInput += Value.Get<FVector2D>() * MouseSensitivity;
}

void AUserArcherCharacter::OnJumpInput()
{
    Jump();
}

void AUserArcherCharacter::StartAiming()
 {
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (!bCanMove || IsDead()) return;
    
    if (GetAmmoCount(CurrentArrowType) <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("화살이 없어서 조준할 수 없습니다!"));
        
        // TODO 화살 부족 ui나 사운드
        return; 
    }
    
    bIsAiming = true;
    SetAiming(true);
    ShowReticle();

    ABow* Bow = Cast<ABow>(EquippedWeapon);

    UE_LOG(LogTemp, Log, TEXT("AimStart"));
    if (Bow)
    {
        Bow->StartAim();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EquippedWeapon is NULL or not a Bow"));
    }
     
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    
 }

void AUserArcherCharacter::StartCharging()
{
    if (IsPlayerManualInputBlockedByCurse()) return;

    if (!EquippedWeapon) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow || !IsAiming()) return;


    if (Bow)
    {
        Bow->StartDraw();
    }
    
    //UE_LOG(LogTemp, Log, TEXT("Charging started."));
}

void AUserArcherCharacter::ReleaseArrow()
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (!EquippedWeapon) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow) return;

    if (!Bow->IsCharging() && !IsAiming())
    {
        SetAiming(false);
        return;
    }
        
    
    Bow->EndDraw();
}

void AUserArcherCharacter::StopTiredShake()
{
    if (!bTiredShakeActive || !TiredCameraShakeClass) return;
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->PlayerCameraManager)
            PC->PlayerCameraManager->StopAllInstancesOfCameraShake(TiredCameraShakeClass, true);
    }
    bTiredShakeActive = false;
}

void AUserArcherCharacter::StopAiming()
{
    UE_LOG(LogTemp, Log, TEXT("Aim Stop"));
    bIsAiming = false;
    SetAiming(false);
    HideReticle();
    StopTiredShake();

    if (EquippedWeapon)
    {
        ABow* Bow = Cast<ABow>(EquippedWeapon);
        if (Bow)
        {
            if (Bow->IsCharging() || bIsAiming)
            {
                ServerPlayCancelMontage();
                Bow->StopAim();
            }
        }
    }
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}


void AUserArcherCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const bool bFocusCurseTarget = ShouldFocusCurseAttackTarget();

    // LookAt을 발사(TryCurseAutoFire)보다 먼저 — 리슨 호스트에서 ControlRotation·에임 정합
    if (IsLocallyControlled() && bFocusCurseTarget)
    {
        ApplyCursedAttackAllyLookFocus(DeltaTime);
    }

    if (HasAuthority() && IsCursedAndAlive())
    {
        CursedBrainTick();
    }

    if (IsCursedAndAlive() && !HasAuthority() && IsLocallyControlled())
    {
        ApplyCursedAutomatedMovementInput();
    }

    if (IsCursedAndAlive() && bDrawCurseDebug)
    {
        DrawCurseDebugVisuals();
    }

    if (IsLocallyControlled())
    {
        if (!bFocusCurseTarget)
        {
            // 마우스 입력 스무딩: 이전 프레임 값과 lerp 후 적용, 약간의 지연감/관성 연출
            SmoothedLookInput = FVector2D(
                FMath::FInterpTo(SmoothedLookInput.X, RawLookInput.X, DeltaTime, LookSmoothingSpeed),
                FMath::FInterpTo(SmoothedLookInput.Y, RawLookInput.Y, DeltaTime, LookSmoothingSpeed)
            );
            AddControllerYawInput(SmoothedLookInput.X);
            AddControllerPitchInput(SmoothedLookInput.Y);
            RawLookInput = FVector2D::ZeroVector;
        }

        ABow* Bow = Cast<ABow>(EquippedWeapon);
        bool bShouldShake = TiredCameraShakeClass && Bow && Bow->IsCharging() && Bow->GetChargeTime() > Bow->GetTiredThreshold();

        if (bShouldShake != bTiredShakeActive)
        {
            if (APlayerController* PC = Cast<APlayerController>(GetController()))
            {
                if (PC->PlayerCameraManager)
                {
                    if (bShouldShake)
                        PC->PlayerCameraManager->StartCameraShake(TiredCameraShakeClass);
                    else
                        PC->PlayerCameraManager->StopAllInstancesOfCameraShake(TiredCameraShakeClass, true);
                }
            }
            bTiredShakeActive = bShouldShake;
        }
    }

    bool bAiming = IsAiming();
    
    float TargetFOV = bAiming ? AimFOV : NormalFOV;
    float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed);
    FollowCamera->SetFieldOfView(NewFOV);
    
    // 마지막 30초 도깨비 위치 공개
    if (AArrowGameState* GS = GetWorld()->GetGameState<AArrowGameState>())
    {
        if (GS->bLastThirtySeconds != bArcherVisionActive)
        {
            bArcherVisionActive = GS->bLastThirtySeconds;
            ApplyDokkaebiVision(bArcherVisionActive);
        }
    }

    //--화살 줍기 관련 로직---//
    AActor* ClosestActor = nullptr;
    float MinDist = 999999.f;

    // 리스트를 돌며 가장 가까운 '박힌 화살'이나 '아이템'을 찾습니다.
    for (int32 i = OverlappingActors.Num() - 1; i >= 0; --i)
    {
        AActor* Actor = OverlappingActors[i];
        if (!IsValid(Actor)) { OverlappingActors.RemoveAt(i); continue; }

        bool bIsPickable = false;

        if (AArrowProjectile* Proj = Cast<AArrowProjectile>(Actor))
        {
            // 화살은 박혀있을 때만 타겟이 됩니다.
            if (Proj->IsStuck()) bIsPickable = true;
        }
        else if (Actor->IsA(AArrowItem::StaticClass()) || Actor -> IsA(ABowItem::StaticClass()))
        {
            // 아이템은 언제나 타겟이 됩니다.
            bIsPickable = true;
        }

        if (bIsPickable)
        {
            float Dist = GetDistanceTo(Actor);
            if (Dist < MinDist)
            {
                MinDist = Dist;
                ClosestActor = Actor;
            }
        }
    }

    // 하이라이트 교체
    if (CurrentTargetActor != ClosestActor)
    {
        UpdateHighlight(CurrentTargetActor, false);
        CurrentTargetActor = ClosestActor;
        UpdateHighlight(CurrentTargetActor, true);
    }
}


//----------Sprint(달리기) 관련 함수-----------------//
void AUserArcherCharacter::OnWalkSlowStarted(const FInputActionValue& Value)
{

    if (IsPlayerManualInputBlockedByCurse()) return;
    // 1. 내 화면(로컬)에서는 답답하지 않게 즉시 속도를 줄임

    // 2. 🔥 내가 클라이언트라면, 서버한테도 내 속도를 깎아달라고 요청!

    if (IsAiming()) return;
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
  
    if (!HasAuthority())
    {
        ServerSetMaxWalkSpeed(RunSpeed);
    }
}

void AUserArcherCharacter::OnWalkSlowEnded(const FInputActionValue& Value)
{

    if (IsPlayerManualInputBlockedByCurse()) return;
    // 1. 내 화면(로컬)에서 즉시 원래 속도로 복구

    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    if (!HasAuthority())
    {
        ServerSetMaxWalkSpeed(NormalSpeed);
    }
}

// 서버가 이 요청을 받아서 실제로 실행하는 부분
void AUserArcherCharacter::ServerSetMaxWalkSpeed_Implementation(float NewSpeed)
{
    // 서버측 캐릭터의 걷기 속도도 똑같이 맞춰줌
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}


//----------Roll(구르기) 관련 함수-----------------//
void AUserArcherCharacter::Roll()
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (IsAiming()) return;
    if (bIsRolling || bIsDead || !bCanMove) return;
    if (!RollMontage) return;

    if (GetRollCooldownRemaining() > 0.0f) return;
    
    bIsRolling = true;
    bCanMove = false;
    const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
    const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
    NextRollAvailableTime = NowServer + FMath::Max(0.01f, RollCooldownDuration);

    PlayMontage(RollMontage, 1.f);

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AUserArcherCharacter::OnRollEnd);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
    }

    // OnRollEnd가 안 불리는 경우를 대비한 안전 타이머
    float SafetyTime = RollMontage->GetPlayLength() + 0.5f;
    GetWorldTimerManager().SetTimer(RollSafetyTimerHandle, this, &AUserArcherCharacter::OnRollSafetyTimeout, SafetyTime, false);
    
    ServerPlayRoll();
}

void AUserArcherCharacter::ServerPlayRoll_Implementation()
{
    MulticastPlayRoll();
}

void AUserArcherCharacter::MulticastPlayRoll_Implementation()
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
            EndDelegate.BindUObject(this, &AUserArcherCharacter::OnRollEnd);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
        }
    }
}

void AUserArcherCharacter::OnRollEnd(UAnimMontage* Montage, bool bInterrupted)
{
    GetWorldTimerManager().ClearTimer(RollSafetyTimerHandle);
	bIsRolling = false;
	bCanMove = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AUserArcherCharacter::OnRollSafetyTimeout()
{
    if (bIsRolling)
    {
        UE_LOG(LogTemp, Warning, TEXT("Roll safety timeout: force resetting roll state"));
        bIsRolling = false;
        bCanMove = true;
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
}

float AUserArcherCharacter::GetRollCooldownRemaining() const
{
    if (!GetWorld()) return 0.0f;
    const AGameStateBase* GS = GetWorld()->GetGameState();
    const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
    return FMath::Max(0.0f, NextRollAvailableTime - NowServer);
}

//-----------화살 관련 -----------//

void AUserArcherCharacter::Input_CycleArrow(const FInputActionValue& Value)
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (IsAiming()) return;
    float ScrollValue = Value.Get<float>();
    if (ScrollValue == 0.f) return;

    int32 CurrentIndex = static_cast<int32>(CurrentArrowType);
    int32 MaxIndex = static_cast<int32>(EArrowType::Max);
    int32 Direction = (ScrollValue > 0.f) ? 1 : -1;

    for (int32 i = 1; i < MaxIndex; i++)
    {
        int32 NextIndex = (CurrentIndex + Direction * i + MaxIndex * MaxIndex) % MaxIndex;
        EArrowType NextType = static_cast<EArrowType>(NextIndex);
        if (GetAmmoCount(NextType) > 0)
        {
            ServerChangeArrowType(NextType);
            return;
        }
    }
}

//----------화살 줍기 관련-------//
//센서에 무언가 들어왔을 때
void AUserArcherCharacter::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("Something Overlapped!"));
    
    if (!OtherActor) return;
    
    // 화살이든 아이템이든 일단 내 범위에 들어오면 리스트에 넣습니다. (상태 체크는 Tick에서!)
    if (OtherActor->IsA(AArrowProjectile::StaticClass()) || OtherActor->IsA(AArrowItem::StaticClass())
        || OtherActor->IsA(ABowItem::StaticClass()))
    {
        OverlappingActors.AddUnique(OtherActor);
        UE_LOG(LogTemp, Log, TEXT("후보 추가됨: %s"), *OtherActor->GetName());
    }
}

//센서에서 무언가 나갔을 때
void AUserArcherCharacter::OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("OnInteractionOverlapEnd!"));
    if (OtherActor)
    {
        OverlappingActors.Remove(OtherActor);
        // 나가는 애가 현재 타겟이었다면 하이라이트 끄기
        if (CurrentTargetActor == OtherActor)
        {
            UpdateHighlight(CurrentTargetActor, false);
            CurrentTargetActor = nullptr;
        }
    }
}

void AUserArcherCharacter::UpdateHighlight(AActor* Target, bool bEnable)
{
    if (!Target) return;

    if (AArrowProjectile* Proj = Cast<AArrowProjectile>(Target))
    {
        if (Proj->ArrowMesh)
        {
            Proj->ArrowMesh->SetRenderCustomDepth(bEnable);
            Proj->ArrowMesh->SetCustomDepthStencilValue(bEnable ? 1 : 0);
        }
    }
    else if (AArrowItem* Item = Cast<AArrowItem>(Target))
    {
        if (Item->ItemMesh)
        {
            Item->ItemMesh->SetRenderCustomDepth(bEnable);
            Item->ItemMesh->SetCustomDepthStencilValue(bEnable ? 1 : 0);
        }
    }else if (ABowItem* BItem = Cast<ABowItem>(Target))
    {
        if (BItem->ItemMesh)
        {
            BItem->ItemMesh->SetRenderCustomDepth(bEnable);
            BItem->ItemMesh->SetCustomDepthStencilValue(bEnable ? 1 : 0);
        }
    }
}

void AUserArcherCharacter::Input_Interact(const FInputActionValue& Value)
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (IsAiming()) return;
    // 1. 이미 실시간(Overlap)으로 찾은 '가장 가까운 타겟'이 있는지 확인합니다.
    if (CurrentTargetActor)
    {
        UE_LOG(LogTemp, Log, TEXT("상호작용 시도: %s"), *CurrentTargetActor->GetName());
        // 2. (선택) 줍는 애니메이션이 있다면 여기서 재생
        // PlayAnimMontage(PickupMontage);

        // 3. 서버에 "나 이거 주울래!"라고 요청 (기존 Server_Interact 활용)
        Server_Interact(CurrentTargetActor);

        // 4. 주웠으므로 하이라이트를 즉시 끄고 타겟을 비워줍니다. (반응 속도 향상)
        // 줍기 성공 후 서버에서 Destroy 되기 전, 클라이언트에서 미리 꺼주는 게 깔끔합니다.
        UpdateHighlight(CurrentTargetActor, false);
        OverlappingActors.Remove(CurrentTargetActor);
        CurrentTargetActor = nullptr;

        
    }
    else
    {
        // 주변에 주울 수 있는 게 없을 때의 피드백 (디버그용)
        UE_LOG(LogTemp, Warning, TEXT("주변에 상호작용 가능한 대상이 없습니다."));
    }
}


void AUserArcherCharacter::Server_Interact_Implementation(AActor* HitActor)
{
    if (!HitActor) return;

    // 타겟이 맵에 스폰된 화살 뭉치인가?
    if (AArrowItem* Item = Cast<AArrowItem>(HitActor))
    {
        Item->PickUp(this);
    }
    // 타겟이 내가 쏴서 박힌 화살인가?
    else if (AArrowProjectile* Proj = Cast<AArrowProjectile>(HitActor))
    {
        Proj->PickUp(this);
    }else if (ABowItem* BItem = Cast<ABowItem>(HitActor))
    {
        BItem->PickUp(this);
    }
}

void AUserArcherCharacter::EquipNewBow(TSubclassOf<ABow> NewBowClass)
{
    if (!NewBowClass) return;
    
    if (EquippedWeapon)
    {
        EquippedWeapon -> Destroy();
        EquippedWeapon = nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    
    ABow* NewBow = GetWorld()->SpawnActor<ABow>(NewBowClass, GetActorLocation(), GetActorRotation(), SpawnParams);
    if (NewBow)
    {
        EquipWeapon(NewBow);
        
        UE_LOG(LogTemp, Log, TEXT("새로운 활 장착 완료: %s"), *NewBow->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("새로운 활 장착 실패"));
    }
}

void AUserArcherCharacter::OnCrouchStarted(const FInputActionValue& Value)
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    if (IsAiming()) return;
    if (!bCanMove || IsDead()) return;
    Crouch();
}
void AUserArcherCharacter::OnCrouchEnded(const FInputActionValue& Value)
{
    if (IsPlayerManualInputBlockedByCurse()) return;
    UnCrouch();
}

void AUserArcherCharacter::ShowReticle()
{
    if (!IsLocallyControlled() || !ReticleWidgetClass) return;

    if (!ReticleWidget)
    {
        ReticleWidget = CreateWidget<UBowReticleWidget>(GetWorld(), ReticleWidgetClass);
        if (ReticleWidget)
        {
            ReticleWidget->InitReticle(this);
        }
    }

    if (ReticleWidget && !ReticleWidget->IsInViewport())
    {
        ReticleWidget->AddToViewport();
    }
}

void AUserArcherCharacter::HideReticle()
{
    if (ReticleWidget)
    {
        ReticleWidget->RemoveFromParent();
    }
}

void AUserArcherCharacter::ApplyCurseControl(float Duration, ADokkaebiCharacter* CurseSource)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(CurseSource))
    {
        UE_LOG(LogTemp, Error, TEXT("[Curse] ApplyCurseControl aborted: CurseSource is null"));
        return;
    }

    CursedDokkaebi = CurseSource;
    LastCurseAutoFireTime = -1.0e9f;
    CurseMoveDebugLastLogTime = -1.0e9f;
    LastCurseBehaviorReevaluateTime = -1.0e9f;

    ResolveAndApplyCurseBehaviorMode(true);

    bIsCursedControl = true;

    // 원격 클라 소유 궁수: 서버 AddMovementInput이 예측/보정에 묻히지 않게 (에디터 단일 설정으로는 한계)
    ForceNetUpdate();
    FlushNetDormancy();
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bIgnoreClientMovementErrorChecksAndCorrection = true;
        // 도망 가속이 먹으려면 걷기/낙하 모드와 양의 보행 속도가 필요할 때가 많음
        if (Move->MovementMode != MOVE_Walking && Move->MovementMode != MOVE_Falling)
        {
            Move->SetMovementMode(MOVE_Walking);
        }
        Move->MaxWalkSpeed = FMath::Max(1.f, FMath::Max(Move->MaxWalkSpeed, NormalSpeed));
    }

    ApplyCurseAimingForBehaviorMode();
    UE_LOG(LogTemp, Warning, TEXT("[Curse] Start %.2fs"), Duration);
    
    UpdateCurseLocalPostProcessVignette();
    
    GetWorldTimerManager().ClearTimer(CurseEndTimerHandle);
    GetWorldTimerManager().SetTimer(
        CurseEndTimerHandle,
        this,
        &AUserArcherCharacter::EndCurseControl,
        FMath::Max(0.1f, Duration),
        false
    );
}

void AUserArcherCharacter::EndCurseControl()
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (!bIsCursedControl)
    {
        return;
    }

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bIgnoreClientMovementErrorChecksAndCorrection = false;
    }

    StopAiming();

    bIsCursedControl = false;
    CursedDokkaebi = nullptr;
    CursedBehaviorMode = ECurseBehaviorMode::FleeFromDokkaebi;
    CursedAttackTarget = nullptr;
    
    UpdateCurseLocalPostProcessVignette();
    
    UE_LOG(LogTemp, Warning, TEXT("[Curse] End"));
}

void AUserArcherCharacter::ApplyCurseAimingForBehaviorMode()
{
    if (!HasAuthority())
    {
        return;
    }

    if (IsAttackAllyCurseMode())
    {
        SetAiming(true);
        if (IsLocallyControlled())
        {
            ApplyCurseAttackAimingLocalVisuals();
        }
        return;
    }

    StopAiming();
}

void AUserArcherCharacter::ApplyCurseAttackAimingLocalVisuals()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    ShowReticle();

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = WalkSpeed;
    }
}

void AUserArcherCharacter::OnRep_IsCursedControl()
{
    // ApplyCurseControl/EndCurseControl의 UE_LOG는 HasAuthority()라 서버 로그에만 찍힘.
    // 클라 창에서 저주 수신 여부는 여기로 확인.
    UE_LOG(LogTemp, Warning, TEXT("[Curse] OnRep bIsCursedControl=%d HasAuthority=%d NetMode=%d"),
        bIsCursedControl ? 1 : 0,
        HasAuthority() ? 1 : 0,
        (int32)GetNetMode());

    if (bIsCursedControl)
    {
        if (IsLocallyControlled())
        {
            if (IsAttackAllyCurseMode())
            {
                ApplyCurseAttackAimingLocalVisuals();
            }
            else
            {
                StopAiming();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("[Curse] OnRep Mode=%d Target=%s Aiming=%d"),
            static_cast<int32>(CursedBehaviorMode),
            CursedAttackTarget ? *CursedAttackTarget->GetName() : TEXT("none"),
            IsAiming() ? 1 : 0);
    }
    else if (IsLocallyControlled())
    {
        StopAiming();
    }

    UpdateCurseLocalPostProcessVignette();
}

FVector AUserArcherCharacter::GetCurseEyeWorldLocation(const AActor* Actor) const
{
    return Actor
        ? Actor->GetActorLocation() + FVector(0.f, 0.f, CursedAllyTraceEyeHeight)
        : FVector::ZeroVector;
}

bool AUserArcherCharacter::IsCurseAllyWithinAttackRange(const ACharacter* Candidate) const
{
    if (!IsCurseAllyArcher(Candidate))
    {
        return false;
    }

    const float DistSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
    if (DistSq > FMath::Square(CursedAllySearchRadius))
    {
        return false;
    }

    return HasClearCurseLineOfSightTo(Candidate);
}

bool AUserArcherCharacter::IsCurseAllyArcher(const ACharacter* Candidate) const
{
    if (!Candidate || Candidate == this)
    {
        return false;
    }

    const AUserArcherCharacter* AllyArcher = Cast<AUserArcherCharacter>(Candidate);
    if (!AllyArcher || AllyArcher->IsDead())
    {
        return false;
    }

    const AArrowPlayerState* MyPS = GetPlayerState<AArrowPlayerState>();
    const AArrowPlayerState* AllyPS = AllyArcher->GetPlayerState<AArrowPlayerState>();
    if (!MyPS || !AllyPS)
    {
        return false;
    }

    // 궁수끼리만 팀 (도깨비 제외)
    return !MyPS->IsDokkaebi() && !AllyPS->IsDokkaebi();
}

bool AUserArcherCharacter::HasClearCurseLineOfSightTo(const ACharacter* Candidate) const
{
    if (!Candidate || !GetWorld())
    {
        return false;
    }

    const FVector Start = GetCurseEyeWorldLocation(this);
    const FVector End = GetCurseEyeWorldLocation(Candidate);
    const float DistToTarget = FVector::Dist(Start, End);
    if (DistToTarget <= KINDA_SMALL_NUMBER)
    {
        return true;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(CurseAllyLOS), false, this);
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        CursedAllyLineTraceChannel,
        Params);

    auto ResolveHitCharacter = [](const FHitResult& InHit) -> const ACharacter*
    {
        if (const ACharacter* AsChar = Cast<ACharacter>(InHit.GetActor()))
        {
            return AsChar;
        }
        if (InHit.GetComponent())
        {
            return Cast<ACharacter>(InHit.GetComponent()->GetOwner());
        }
        return nullptr;
    };

    // Visibility는 Pawn에 Block 안 하는 설정이 많음 → 히트 없음 = 벽 없음
    if (!bHit)
    {
        return true;
    }

    const ACharacter* HitChar = ResolveHitCharacter(Hit);
    if (HitChar == Candidate)
    {
        return true;
    }

    const float HitDist = FVector::Dist(Start, Hit.ImpactPoint);
    constexpr float BlockToleranceCm = 25.f;
    if (HitDist < DistToTarget - BlockToleranceCm)
    {
        return false;
    }

    return true;
}

bool AUserArcherCharacter::TryResolveCurseAllyTarget(ACharacter*& OutTarget) const
{
    OutTarget = nullptr;

    if (!HasAuthority() || bIsDead)
    {
        return false;
    }

    const UWorld* World = GetWorld();
    const AGameStateBase* GS = World ? World->GetGameState() : nullptr;
    if (!GS)
    {
        return false;
    }

    const FVector MyLoc = GetActorLocation();

    ACharacter* BestTarget = nullptr;
    float BestDistSq = TNumericLimits<float>::Max();

    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS)
        {
            continue;
        }

        APawn* Pawn = PS->GetPawn();
        ACharacter* Candidate = Cast<ACharacter>(Pawn);
        if (!IsCurseAllyArcher(Candidate))
        {
            if (bLogCurseAllyResolve && Candidate)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Curse][Resolve] skip %s: not ally archer"), *Candidate->GetName());
            }
            continue;
        }

        const float DistSq = FVector::DistSquared2D(MyLoc, Candidate->GetActorLocation());
        if (!IsCurseAllyWithinAttackRange(Candidate))
        {
            if (bLogCurseAllyResolve)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Curse][Resolve] skip %s: out of range or LOS"), *Candidate->GetName());
            }
            continue;
        }

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestTarget = Candidate;
        }
    }

    if (!BestTarget)
    {
        if (bLogCurseAllyResolve)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Curse][Resolve] no valid ally (PlayerArray=%d)"), GS->PlayerArray.Num());
        }
        return false;
    }

    OutTarget = BestTarget;
    return true;
}

bool AUserArcherCharacter::TryGetCursedHorizontalMoveDirection2D(
    const FVector& GoalWorldLocation,
    const bool bMoveAwayFromGoal,
    FVector& OutDir) const
{
    OutDir = FVector::ZeroVector;

    FVector Delta = bMoveAwayFromGoal
        ? (GetActorLocation() - GoalWorldLocation)
        : (GoalWorldLocation - GetActorLocation());
    Delta.Z = 0.f;
    OutDir = Delta.GetSafeNormal();
    return !OutDir.IsNearlyZero();
}

bool AUserArcherCharacter::TryGetCursedMovementWorldDirection2D(FVector& OutDir) const
{
    OutDir = FVector::ZeroVector;

    if (!IsCursedAndAlive())
    {
        return false;
    }

    if (IsAttackAllyCurseMode() && IsCurseAllyArcher(CursedAttackTarget))
    {
        return TryGetCursedHorizontalMoveDirection2D(CursedAttackTarget->GetActorLocation(), false, OutDir);
    }

    const ADokkaebiCharacter* Dok = CursedDokkaebi;
    if (!IsValid(Dok))
    {
        return false;
    }

    return TryGetCursedHorizontalMoveDirection2D(Dok->GetActorLocation(), true, OutDir);
}

void AUserArcherCharacter::ApplyCursedAutomatedMovementInput()
{
    FVector Dir;
    if (!TryGetCursedMovementWorldDirection2D(Dir))
    {
        return;
    }

    const float Scale = ShouldFocusCurseAttackTarget() ? CursedApproachInputScale : CursedFleeInputScale;

    AddMovementInput(Dir, Scale, true);
}

bool AUserArcherCharacter::ShouldFocusCurseAttackTarget() const
{
    return IsCursedAndAlive() && IsAttackAllyCurseMode() && IsCurseAllyArcher(CursedAttackTarget);
}

void AUserArcherCharacter::ApplyCursedAttackAllyLookFocus(float DeltaTime)
{
    AController* ControllerPtr = GetController();
    if (!ControllerPtr || !IsCurseAllyArcher(CursedAttackTarget))
    {
        return;
    }

    const FVector ViewStart = GetCurseEyeWorldLocation(this);
    const FVector ViewEnd = GetCurseEyeWorldLocation(CursedAttackTarget);
    const FRotator TargetRotation = (ViewEnd - ViewStart).Rotation();

    if (CursedAllyLookInterpSpeed <= 0.f)
    {
        ControllerPtr->SetControlRotation(TargetRotation);
    }
    else
    {
        const FRotator NewRotation = FMath::RInterpTo(
            ControllerPtr->GetControlRotation(),
            TargetRotation,
            DeltaTime,
            CursedAllyLookInterpSpeed);
        ControllerPtr->SetControlRotation(NewRotation);
    }

    RawLookInput = FVector2D::ZeroVector;
    SmoothedLookInput = FVector2D::ZeroVector;
}

void AUserArcherCharacter::ResolveAndApplyCurseBehaviorMode(const bool bLogResolution)
{
    if (!HasAuthority())
    {
        return;
    }

    const ECurseBehaviorMode PreviousMode = CursedBehaviorMode;
    ACharacter* const PreviousTarget = CursedAttackTarget;

    ACharacter* AllyTarget = nullptr;
    if (TryResolveCurseAllyTarget(AllyTarget))
    {
        CursedBehaviorMode = ECurseBehaviorMode::AttackAlly;
        CursedAttackTarget = AllyTarget;

        if (bLogResolution)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Curse] Mode=AttackAlly Target=%s Dist2D=%.0f"),
                *GetNameSafe(AllyTarget),
                AllyTarget ? FVector::Dist2D(GetActorLocation(), AllyTarget->GetActorLocation()) : -1.f);
        }
    }
    else
    {
        CursedBehaviorMode = ECurseBehaviorMode::FleeFromDokkaebi;
        CursedAttackTarget = nullptr;

        if (bLogResolution)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Curse] Mode=FleeFromDokkaebi (no valid ally LOS)"));
        }
    }

    if (PreviousMode != CursedBehaviorMode || PreviousTarget != CursedAttackTarget)
    {
        ApplyCurseAimingForBehaviorMode();

        if (bLogResolution && PreviousMode != CursedBehaviorMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Curse] Behavior changed %d -> %d"),
                static_cast<int32>(PreviousMode),
                static_cast<int32>(CursedBehaviorMode));
        }
    }
}

bool AUserArcherCharacter::IsCurrentCurseAttackTargetValid() const
{
    return IsAttackAllyCurseMode() && IsCurseAllyWithinAttackRange(CursedAttackTarget);
}

void AUserArcherCharacter::ReevaluateCurseBehaviorOnAuthority()
{
    if (!HasAuthority() || !IsCursedAndAlive())
    {
        return;
    }

    if (IsCurrentCurseAttackTargetValid())
    {
        return;
    }

    ResolveAndApplyCurseBehaviorMode(bLogCurseAllyResolve);
}

void AUserArcherCharacter::DrawCurseDebugVisuals() const
{
    UWorld* World = GetWorld();
    if (!World || !IsCursedAndAlive())
    {
        return;
    }

    const FVector ViewStart = GetCurseEyeWorldLocation(this);
    const float DebugLifetime = 0.f;

    if (ShouldFocusCurseAttackTarget())
    {
        const FVector TargetPoint = GetCurseEyeWorldLocation(CursedAttackTarget);
        DrawDebugLine(World, ViewStart, TargetPoint, FColor::Green, false, DebugLifetime, 0, 2.f);
        DrawDebugSphere(World, TargetPoint, 24.f, 8, FColor::Green, false, DebugLifetime);
    }
    else if (IsValid(CursedDokkaebi))
    {
        const FVector DokPoint = GetCurseEyeWorldLocation(CursedDokkaebi);
        const FVector FleeHint = ViewStart + (ViewStart - DokPoint).GetSafeNormal2D() * 200.f;
        DrawDebugLine(World, ViewStart, FleeHint, FColor::Red, false, DebugLifetime, 0, 2.f);
    }

    DrawDebugCircle(
        World,
        GetActorLocation(),
        CursedAllySearchRadius,
        32,
        FColor::Cyan,
        false,
        DebugLifetime,
        0,
        1.5f,
        FVector(1.f, 0.f, 0.f),
        FVector(0.f, 1.f, 0.f),
        false);
}

void AUserArcherCharacter::TryCurseAutoFire()
{
    if (!HasAuthority() || !ShouldFocusCurseAttackTarget())
    {
        return;
    }

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow || Bow->IsReloading() || Bow->IsNocking())
    {
        return;
    }

    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const float Now = World->GetTimeSeconds();
    if (Now - LastCurseAutoFireTime < CurseAutoFireInterval)
    {
        return;
    }

    LastCurseAutoFireTime = Now;
    Bow->ExecuteCurseShot(CurseAutoFireChargePercent, CursedAttackTarget, CursedAllyTraceEyeHeight);
}

void AUserArcherCharacter::CursedBrainTick()
{
    if (!HasAuthority() || !IsCursedAndAlive())
    {
        return;
    }

    const UWorld* World = GetWorld();
    const float Now = World ? World->GetTimeSeconds() : 0.f;

    if (bReevaluateCurseBehaviorWhileActive && World)
    {
        if (Now - LastCurseBehaviorReevaluateTime >= CurseBehaviorReevaluateInterval)
        {
            LastCurseBehaviorReevaluateTime = Now;
            ReevaluateCurseBehaviorOnAuthority();
        }
    }

    if (IsAttackAllyCurseMode())
    {
        TryCurseAutoFire();
    }

    // 원격 클라가 조종하는 궁수: 서버에서 AddMovementInput 해도 ServerMove(클라 정지 입력)에 밀림 → 소유 클라 Tick에서만 입력.
    if (!IsLocallyControlled())
    {
        return;
    }

    // 리슨 서버에서 본인 궁수만 여기서 입력(클라 미러는 !HasAuthority()라 안 돔).
    ApplyCursedAutomatedMovementInput();

    if (World)
    {
        if (Now - CurseMoveDebugLastLogTime >= 0.25f)
        {
            CurseMoveDebugLastLogTime = Now;
            // 같은 틱에서 바로 읽으면 Vel/Acc가 아직 갱신 전일 수 있어, 다음 틱에 한 번 더 찍음
            GetWorldTimerManager().SetTimerForNextTick([this]()
            {
                if (!IsValid(this) || !HasAuthority() || !IsCursedAndAlive())
                {
                    return;
                }
                UCharacterMovementComponent* M = GetCharacterMovement();
                const FVector V = GetVelocity();
                const FVector Acc = M ? M->GetCurrentAcceleration() : FVector::ZeroVector;
                const float MoveScale = ShouldFocusCurseAttackTarget()
                    ? CursedApproachInputScale
                    : CursedFleeInputScale;
                UE_LOG(LogTemp, Warning,
                    TEXT("[Curse] BrainTick+1 %s Vel2D=%.1f Acc2D=%.1f BehaviorMode=%d MaxWalk=%.0f Scale=%.2f"),
                    *GetName(),
                    V.Size2D(),
                    Acc.Size2D(),
                    static_cast<int32>(CursedBehaviorMode),
                    M ? M->MaxWalkSpeed : -1.f,
                    MoveScale);
            });
        }
    }
}

void AUserArcherCharacter::ApplyDokkaebiVision(bool bEnable)
{
    AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
    if (!GS) return;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AArrowPlayerState* ArrowPS = Cast<AArrowPlayerState>(PS);
        if (!ArrowPS || !ArrowPS->IsDokkaebi()) continue;

        ADokkaebiCharacter* Dokkaebi = Cast<ADokkaebiCharacter>(ArrowPS->GetPawn());
        if (!Dokkaebi) continue;

        UMaterialInterface* Mat = bEnable ? Dokkaebi->GetSpiritSightOverlayMaterial() : nullptr;
        if (USkeletalMeshComponent* DokkaebiMesh = Dokkaebi->GetMesh())
        {
            DokkaebiMesh->SetOverlayMaterial(Mat);
        }
    }
}

void AUserArcherCharacter::UpdateCurseLocalPostProcessVignette()
{
    if (!IsLocallyControlled() || !FollowCamera)
    {
        return;
    }
    FollowCamera->PostProcessBlendWeight = bIsCursedControl ? 1.f : 0.f;
}