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

bool AUserArcherCharacter::GetPrimarySkillHudMeta(UTexture2D*& OutIcon, FText& OutKeyText) const
{
    OutIcon = RollSkillIcon;
    OutKeyText = RollSkillKeyText;
    return true;
}

bool AUserArcherCharacter::GetPrimarySkillCooldown(float& OutRemaining, float& OutDuration) const
{
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

void AUserArcherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUserArcherCharacter::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUserArcherCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);

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
}

void AUserArcherCharacter::Move(const FInputActionValue& Value)
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

void AUserArcherCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void AUserArcherCharacter::StartAiming()
 {
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

    if (Bow && (Bow->IsReloading() || Bow->IsNocking()))
    {
        // 재장전 중이면 조준 시도 자체를 무시합니다.
        return;
    }

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

    if (!EquippedWeapon) return;

    ABow* Bow = Cast<ABow>(EquippedWeapon);
    if (!Bow || !IsAiming()) return;


    if (Bow)
    {
        if (Bow->IsNocking() || Bow->IsReloading()) return;
        Bow -> StartDraw();
    }
    
    //UE_LOG(LogTemp, Log, TEXT("Charging started."));
}

void AUserArcherCharacter::ReleaseArrow()
{
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

    if (IsLocallyControlled())
    {
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


//----------Walk(걷기) 관련 함수-----------------//
void AUserArcherCharacter::OnWalkSlowStarted(const FInputActionValue& Value)
{
    // 1. 내 화면(로컬)에서는 답답하지 않게 즉시 속도를 줄임
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // 2. 🔥 내가 클라이언트라면, 서버한테도 내 속도를 깎아달라고 요청!
    if (!HasAuthority())
    {
        ServerSetMaxWalkSpeed(WalkSpeed);
    }
}

void AUserArcherCharacter::OnWalkSlowEnded(const FInputActionValue& Value)
{
    // 1. 내 화면(로컬)에서 즉시 원래 속도로 복구
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

    // 2.서버한테도 내 속도 원래대로 돌려달라고 요청!
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
	bIsRolling = false;
	bCanMove = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
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
    float ScrollValue = Value.Get<float>();
    if (ScrollValue == 0.f) return;

    // 현재 Enum 값을 정수로 변환
    int32 CurrentIndex = static_cast<int32>(CurrentArrowType);
    int32 MaxIndex = static_cast<int32>(EArrowType::Max);

    // 휠 방향에 따라 다음/이전 무기 계산 (나머지 연산자로 뱅글뱅글 돌게 만듦)
    if (ScrollValue > 0.f)
    {
        CurrentIndex = (CurrentIndex + 1) % MaxIndex;
    }
    else
    {
        CurrentIndex = (CurrentIndex - 1 + MaxIndex) % MaxIndex;
    }
    
    EArrowType NewType = static_cast<EArrowType>(CurrentIndex);

    // 🚀 서버에 화살 교체 요청! (아까 만들어둔 함수 재활용)
    ServerChangeArrowType(NewType);
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
    if (!bCanMove || IsDead()) return;
    Crouch();
}
void AUserArcherCharacter::OnCrouchEnded(const FInputActionValue& Value)
{
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
