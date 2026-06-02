#include "DokkaebiCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "ArrowGame/Core/ArrowPlayerState.h"
#include "InputActionValue.h"
#include "../Character/DokkaebiDecoy.h"
#include "Net/UnrealNetwork.h"
#include "../Character/DokkaebiCurseProjectile.h"
#include "../UI/SpiritSightMarkerWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

// --- 복제: 은신 전 클라, 스킬 상태·투시 종료 시각은 시전자(Owner)만 ---

void ADokkaebiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADokkaebiCharacter, bIsStealthed);
	DOREPLIFETIME(ADokkaebiCharacter, bCurseOrbReady);
	DOREPLIFETIME(ADokkaebiCharacter, bCurseOrbHideInstant);
	DOREPLIFETIME(ADokkaebiCharacter, SyncPitch);
	DOREPLIFETIME_CONDITION(ADokkaebiCharacter, SkillStates, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADokkaebiCharacter, SpiritSightEndServerTime, COND_OwnerOnly);
	
}

ADokkaebiCharacter::ADokkaebiCharacter()
{
	PrimaryActorTick.bCanEverTick = true; // 투시 마커 Tick 갱신용
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 60.f);
	CameraBoom->TargetOffset = FVector(0.f, 0.f, 30.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SpiritSightPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("SpiritSightPP"));
	SpiritSightPPComp->SetupAttachment(RootComponent);
	SpiritSightPPComp->bEnabled = true;
	SpiritSightPPComp->BlendWeight = 0.f;  // 기본은 꺼짐
	SpiritSightPPComp->bUnbound = true;
	
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	CurseOrbSlot = CreateDefaultSubobject<USceneComponent>(TEXT("CurseOrbSlot"));
	CurseOrbSlot->SetupAttachment(GetCapsuleComponent());
	
	CurseOrbPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CurseOrbPreview"));
	CurseOrbPreviewMesh->SetupAttachment(CurseOrbSlot);
	CurseOrbPreviewMesh->SetHiddenInGame(true);
	CurseOrbPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}
}

int32 ADokkaebiCharacter::GetSkillSlotCount() const
{
	return SkillSpecs.Num();
	
}

bool ADokkaebiCharacter::GetSkillHudMetaByIndex(int32 SlotIndex, UTexture2D*& OutIcon, FText& OutKeyText) const
{
	if (!SkillSpecs.IsValidIndex(SlotIndex))
	{
		OutIcon = nullptr;
		OutKeyText = FText::GetEmpty();
		return false;
	}
	OutIcon = SkillSpecs[SlotIndex].Icon;
	OutKeyText = SkillSpecs[SlotIndex].KeyText;
	return true;
}

bool ADokkaebiCharacter::GetSkillCooldownByIndex(int32 SlotIndex, float& OutRemaining, float& OutDuration) const
{
	OutRemaining = 0.f;
	OutDuration = 0.01f;
	const EDokkaebiSkillIndex SkillIndex = static_cast<EDokkaebiSkillIndex>(SlotIndex);
	OutRemaining = GetSkillCooldownRemainingByIndex(SkillIndex);
	OutDuration = GetSkillCooldownDurationByIndex(SkillIndex);
	return true;
}

// Hit reaction: CharacterBase -> Multicast_TriggerHitFlinch -> AnimBP HitFlinchAlpha.
// void ADokkaebiCharacter::PlayHitReaction()
// {
// 	if (!HitMontage || bIsDead) return;
// 	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
// 	{
// 		AnimInstance->Montage_Play(HitMontage);
// 	}
// }

void ADokkaebiCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NormalWalkSpeed;
	}
	
	if (SpiritSightPPMaterial)
	{
		SpiritSightPPComp->Settings.WeightedBlendables.Array.Add(
			FWeightedBlendable(1.f, SpiritSightPPMaterial));
	}
	
	SkillStates.SetNum(SkillSpecs.Num());

	EnsureSpiritSightMarkerWidget();
}


void ADokkaebiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		SyncPitch = FRotator::NormalizeAxis(GetControlRotation().Pitch);
	}

	if (!IsLocallyControlled()) return;
	UpdateSpiritSightMarkers(DeltaTime); // 다른 클라는 마커 불필요
}

void ADokkaebiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SpiritSightMarkerWidget)
	{
		SpiritSightMarkerWidget->RemoveFromParent();
		SpiritSightMarkerWidget = nullptr;
	}

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
		
		if (CurseSkillAction)
		{
			EnhancedInput->BindAction(CurseSkillAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::Input_PrepareCurseOrb);
		}
		
		if (SpiritSightAction)
		{
			EnhancedInput->BindAction(SpiritSightAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::Input_SpiritSight);
		}
		
		if (FireAction)
		{
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::FireCurseProjectile);
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

	EnsureSpiritSightMarkerWidget(); // BeginPlay엔 Controller 없을 수 있어 Setup에서도 재시도
}

// 로컬 플레이어만: PC 생긴 뒤 뷰포트에 마커 레이어 올림
void ADokkaebiCharacter::EnsureSpiritSightMarkerWidget()
{
	if (!IsLocallyControlled() || !SpiritSightMarkerWidgetClass || SpiritSightMarkerWidget)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		SpiritSightMarkerWidget = CreateWidget<USpiritSightMarkerWidget>(PC, SpiritSightMarkerWidgetClass);
		if (SpiritSightMarkerWidget)
		{
			if (SpiritSightMarkerEntryClass)
			{
				SpiritSightMarkerWidget->MarkerEntryWidgetClass = SpiritSightMarkerEntryClass;
			}
			SpiritSightMarkerWidget->AddToViewport(10);
			// 뷰포트 전체를 안 쓰면 캔버스가 접혀서 마커가 왼쪽 위에만 보임
			SpiritSightMarkerWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
			SpiritSightMarkerWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
			// SetOffsetsInViewport 는 엔진 버전에 없을 수 있음 — 앵커 풀스크린만으로 보통 충분
			SpiritSightMarkerWidget->SetVisibility(ESlateVisibility::Hidden);
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

	if (!TryCommitSkillUseOnAuthority(DecoyIndex))
	{
		return;
	}
	
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
		else
		{
			Decoy->SyncMovementFromOwner();
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
	
	if (bIsStealthed) return false;

	return CanUseSkillOnAuthority(DecoyIndex);
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

	MulticastPlayDecoyVanishFX(GetActorLocation());
	
	OnRep_IsStealthed();
}

void ADokkaebiCharacter::Input_DecoySkillA(const FInputActionValue& Value)
{
	if (IsLocallyControlled() && bCurseOrbReady)
	{
		RequestCancelCurseOrbPrepare();
	}

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

UStaticMeshComponent* ADokkaebiCharacter::FindMaskMeshOnSocket() const
{
	if (MaskAttachSocketName.IsNone())
	{
		return nullptr;
	}

	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);

	for (UStaticMeshComponent* StaticMesh : StaticMeshes)
	{
		if (!StaticMesh || StaticMesh == CurseOrbPreviewMesh)
		{
			continue;
		}

		if (StaticMesh->GetAttachSocketName() == MaskAttachSocketName)
		{
			return StaticMesh;
		}
	}

	return nullptr;
}

void ADokkaebiCharacter::ApplyStealthOpacityToMesh(UMeshComponent* MeshComp, bool bStealth) const
{
	if (!MeshComp)
	{
		return;
	}

	const float Opacity = bStealth ? 0.3f : 1.0f;
	for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); ++MaterialIndex)
	{
		if (UMaterialInstanceDynamic* MID = MeshComp->CreateDynamicMaterialInstance(MaterialIndex))
		{
			MID->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		}
	}
}

void ADokkaebiCharacter::OnRep_IsStealthed()
{
	TArray<USkeletalMeshComponent*> SkelMeshes;
	GetComponents<USkeletalMeshComponent>(SkelMeshes);

	UStaticMeshComponent* MaskMesh = FindMaskMeshOnSocket();

	// 본인: 반투명+포스트 / 타인: 메시 숨김
	if (IsLocallyControlled())
	{
		for (USkeletalMeshComponent* MeshComp : SkelMeshes)
		{
			ApplyStealthOpacityToMesh(MeshComp, bIsStealthed);
		}

		ApplyStealthOpacityToMesh(MaskMesh, bIsStealthed);

		if (FollowCamera)
		{
			FollowCamera->PostProcessBlendWeight = bIsStealthed ? 1.0f : 0.0f;
		}
	}
	else
	{
		for (USkeletalMeshComponent* MeshComp : SkelMeshes)
		{
			if (MeshComp)
			{
				MeshComp->SetHiddenInGame(bIsStealthed);
			}
		}

		if (MaskMesh)
		{
			MaskMesh->SetHiddenInGame(bIsStealthed);
		}
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

void ADokkaebiCharacter::Input_PrepareCurseOrb(const FInputActionValue& /*Value*/)
{
	if (!IsLocallyControlled() || bIsDead)
	{
		return;
	}

	if (bCurseOrbReady)
	{
		RequestCancelCurseOrbPrepare();
		return;
	}

	constexpr EDokkaebiSkillIndex CurseIndex = EDokkaebiSkillIndex::SkillB;
	if (IsSkillCoolingDownByIndex(CurseIndex))
	{
		return;
	}

	if (HasAuthority())
	{
		Server_StartCursePrepare_Implementation();
	}
	else
	{
		Server_StartCursePrepare();
	}
}

void ADokkaebiCharacter::RequestCancelCurseOrbPrepare()
{
	if (HasAuthority())
	{
		Server_CancelCursePrepare_Implementation();
	}
	else
	{
		Server_CancelCursePrepare();
	}
}

void ADokkaebiCharacter::Server_StartCursePrepare_Implementation()
{
	if (!HasAuthority() || bIsDead || bCurseOrbReady)
	{
		return;
	}

	constexpr EDokkaebiSkillIndex CurseIndex = EDokkaebiSkillIndex::SkillB;
	if (IsSkillCoolingDownByIndex(CurseIndex))
	{
		return;
	}

	SetCurseOrbReadyOnServer(true);
	Multicast_PlayPrepareCurseMontage();
}

void ADokkaebiCharacter::Server_CancelCursePrepare_Implementation()
{
	CancelCurseOrbPrepareOnServer();
}

void ADokkaebiCharacter::CancelCurseOrbPrepareOnServer(bool bInstantHide)
{
	if (!HasAuthority() || !bCurseOrbReady)
	{
		return;
	}

	SetCurseOrbReadyOnServer(false, bInstantHide);
	Multicast_StopPrepareCurseMontage();
}

void ADokkaebiCharacter::Multicast_PlayPrepareCurseMontage_Implementation()
{
	PlayPrepareCurseMontage();
}

void ADokkaebiCharacter::Multicast_PlayFireCurseMontage_Implementation()
{
	PlayFireCurseMontage();
}

void ADokkaebiCharacter::Multicast_StopPrepareCurseMontage_Implementation()
{
	StopPrepareCurseMontage();
}

void ADokkaebiCharacter::FireCurseProjectile()
{
	if (!IsLocallyControlled() || bIsDead || !bCurseOrbReady)
	{
		return;
	}

	FVector SpawnLoc, AimDir;
	ComputeCurseFireFromView(SpawnLoc, AimDir);
	AimDir = AimDir.GetSafeNormal();

	if (HasAuthority())
	{
		Server_FireCurseProjectile_Implementation(SpawnLoc, AimDir);
	}
	else
	{
		Server_FireCurseProjectile(SpawnLoc, AimDir);
	}
}

void ADokkaebiCharacter::Server_FireCurseProjectile_Implementation(FVector SpawnLoc, FVector AimDir)
{
	if (!bCurseOrbReady)
	{
		return;
	}

	constexpr int32 CurseIndex = 1;
	if (!TryCommitSkillUseOnAuthority(CurseIndex))
	{
		return;
	}

	if (!CurseProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CurseProjectileClass is null"));
		return;
	}

	AimDir = AimDir.GetSafeNormal();
	if (AimDir.IsNearlyZero())
	{
		AimDir = GetActorForwardVector();
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator SpawnRot = AimDir.Rotation();

	ADokkaebiCurseProjectile* Proj = GetWorld()->SpawnActor<ADokkaebiCurseProjectile>(
		CurseProjectileClass, SpawnLoc, SpawnRot, Params);

	if (!Proj)
	{
		return;
	}

	Proj->SetCurseCaster(this);
	Proj->SetInstigator(this);
	Proj->SetOwner(this);

	if (UProjectileMovementComponent* Move = Proj->ProjectileMovement)
	{
		const float Speed = Move->InitialSpeed > 0.f ? Move->InitialSpeed : 1800.f;
		Move->Velocity = AimDir * Speed;
	}

	Multicast_PlayFireCurseMontage();
	CancelCurseOrbPrepareOnServer(true);
}

bool ADokkaebiCharacter::ComputeCurseFireFromView(FVector& OutSpawnLoc, FVector& OutAimDir) const
{
	OutAimDir = GetActorForwardVector();
	OutSpawnLoc = GetActorLocation();
	
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		if (CurseOrbSlot)
		{
			OutSpawnLoc = CurseOrbSlot->GetComponentLocation();
		}
		return false;
	}
	
	FVector CamLoc;
	FRotator CamRot;
	
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	OutAimDir = CamRot.Vector();
	
	if (CurseOrbSlot)
	{
		OutSpawnLoc = CurseOrbSlot->GetComponentLocation();
	}
	else
	{
		OutSpawnLoc = CamLoc + CamRot.Vector() * 80.f;
	}

	return true;
}

bool ADokkaebiCharacter::CanUseSkillOnAuthority(int32 SkillIndex) const
{
	if (!HasAuthority()) return false;
	if (bIsDead) return false;
	if (!SkillStates.IsValidIndex(SkillIndex)) return false;
	if (!SkillSpecs.IsValidIndex(SkillIndex)) return false;
	
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	
	const FSkillRuntimeState& State = SkillStates[SkillIndex];
	if (State.bInputLocked) return false;
	if (NowServer < State.NextAvailableTime) return false;
	
	return true;
}
// 쿨다운·입력 락 갱신 — 스킬별 실제 효과는 호출부에서
bool ADokkaebiCharacter::TryCommitSkillUseOnAuthority(int32 SkillIndex)
{
	if (!CanUseSkillOnAuthority(SkillIndex)) return false;
	
	FSkillRuntimeState& State = SkillStates[SkillIndex];
	const FSkillSpec& Spec = SkillSpecs[SkillIndex];
	
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	
	State.NextAvailableTime = NowServer + FMath::Max(0.01f, Spec.Cooldown);
	State.bInputLocked = true;
	
	FTimerHandle TempHandle;
	GetWorldTimerManager().SetTimer(
		TempHandle,
		[this, SkillIndex]()
		{
			if (SkillStates.IsValidIndex(SkillIndex))
			{
				SkillStates[SkillIndex].bInputLocked = false;
			}
		},
		FMath::Max(0.01f, Spec.InputLockDuration),
		false
	);
	return true;
}

void ADokkaebiCharacter::UnlockSkillInput(int32 SkillIndex)
{
	if (SkillStates.IsValidIndex(SkillIndex))
	{
		SkillStates[SkillIndex].bInputLocked = false;
	}
}

void ADokkaebiCharacter::OnRep_SpiritSightEnd()
{
	EnsureSpiritSightMarkerWidget(); // 복제 직후 위젯 없으면 생성
}

bool ADokkaebiCharacter::IsSpiritSightActive_ServerTime() const
{
	if (!GetWorld()) return false;
	const AGameStateBase* GS = GetWorld()->GetGameState();
	const float Now = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	return SpiritSightEndServerTime > 0.f && Now < SpiritSightEndServerTime;
}

void ADokkaebiCharacter::Input_SpiritSight(const FInputActionValue& Value)
{
	if (IsLocallyControlled() && bCurseOrbReady)
	{
		RequestCancelCurseOrbPrepare();
	}

	// Listen 호스트는 RPC 없이 권한 함수만 태움 (미끼와 동일)
	if (HasAuthority())
	{
		ExecuteSpiritSightOnAuthority();
	}
	else
	{
		Server_UseSpiritSight();
	}
}

void ADokkaebiCharacter::Server_UseSpiritSight_Implementation()
{
	ExecuteSpiritSightOnAuthority();
}

void ADokkaebiCharacter::ExecuteSpiritSightOnAuthority()
{
	constexpr int32 SpiritSightIndex = 2; // SkillSpecs[2] 쿨다운
	if (!TryCommitSkillUseOnAuthority(SpiritSightIndex))
	{
		return;
	}

	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	const float NowServer = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	SpiritSightEndServerTime = NowServer + FMath::Max(0.1f, SpiritSightDuration);
}

void ADokkaebiCharacter::UpdateSpiritSightMarkers(float DeltaTime)
{
	if (!SpiritSightMarkerWidget)
	{
		return;
	}

	if (SpiritSightPPComp)
	{
		SpiritSightPPComp->BlendWeight = IsSpiritSightActive_ServerTime() ? 1.f : 0.f;
	}
	
	if (!IsSpiritSightActive_ServerTime())
	{
		SpiritSightMarkerWidget->SetVisibility(ESlateVisibility::Hidden);
		SpiritSightMarkerWidget->SetSpiritMarkerDrawInfos(TArray<FSpiritSightMarkerDrawInfo>());
		
		// 투시 종료: 적 오버레이 머티리얼 해제
		if (AGameStateBase* OffGS = GetWorld()->GetGameState())
		{
			AArrowPlayerState* OffMyPS = GetPlayerState<AArrowPlayerState>();
			for (APlayerState* PS : OffGS->PlayerArray)
			{
				AArrowPlayerState* APS = Cast<AArrowPlayerState>(PS);
				if (!APS || !OffMyPS || APS == OffMyPS) continue;
				if (OffMyPS->IsDokkaebi() == APS->IsDokkaebi()) continue;
				if (APawn* P = APS->GetPawn())
				{
					if (ACharacter* C = Cast<ACharacter>(P))
					{
						if (USkeletalMeshComponent* M = C->GetMesh())
						{
							M->SetOverlayMaterial(nullptr);
						}
					}
				}
			}
		}
		
		return;
	}
	// 아래: IsDokkaebi 다름 = 적, 화면에 보이면 스크린 좌표만 넘김 (벽 가림은 UI가 무시)

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	AArrowPlayerState* MyPS = GetPlayerState<AArrowPlayerState>();
	if (!MyPS)
	{
		return;
	}

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS)
	{
		return;
	}

	FVector RefLoc = GetActorLocation();
	if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
	{
		RefLoc = PCM->GetCameraLocation();
	}

	const float DNear = FMath::Min(SpiritSightScaleNearCm, SpiritSightScaleFarCm);
	const float DFar = FMath::Max(SpiritSightScaleNearCm, SpiritSightScaleFarCm);

	TArray<FSpiritSightMarkerDrawInfo> MarkerInfos;
	for (APlayerState* PS : GS->PlayerArray)
	{
		
		AArrowPlayerState* APS = Cast<AArrowPlayerState>(PS);
		if (!APS || APS == MyPS)
		{
			continue;
		}
		if (MyPS->IsDokkaebi() == APS->IsDokkaebi())
		{
			continue;
		}

		APawn* OtherPawn = APS->GetPawn();
		if (!OtherPawn || OtherPawn->IsActorBeingDestroyed())
		{
			continue;
		}

		// 투시 중: 적 메시에 실루엣 오버레이 (Disable Depth Test라 벽 뒤에서도 보임)
		if (SpiritSightOverlayMaterial)
		{
			if (ACharacter* OtherChar = Cast<ACharacter>(OtherPawn))
			{
				if (USkeletalMeshComponent* EnemyMesh = OtherChar->GetMesh())
				{
					if (EnemyMesh->GetOverlayMaterial() != SpiritSightOverlayMaterial)
					{
						EnemyMesh->SetOverlayMaterial(SpiritSightOverlayMaterial);
					}
				}
			}
		}
		
		if (ACharacterBase* OtherChar = Cast<ACharacterBase>(OtherPawn))
		{
			if (OtherChar->bIsDead)
			{
				continue;
			}
		}

		const FVector WorldLoc = OtherPawn->GetActorLocation() + FVector(0, 0, 80.f);
		FVector2D WidgetSpace;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLoc, WidgetSpace, true))
		{
			const float DistCm = FVector::Dist(RefLoc, WorldLoc);
			const float Scale = FMath::GetMappedRangeValueClamped(FVector2D(DNear, DFar), FVector2D(SpiritSightScaleAtNear, SpiritSightScaleAtFar), DistCm);

			FSpiritSightMarkerDrawInfo Info;
			Info.ScreenPosition = WidgetSpace;
			Info.UniformScale = Scale;
			MarkerInfos.Add(Info);
		}
	}

	SpiritSightMarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	SpiritSightMarkerWidget->SetSpiritMarkerDrawInfos(MarkerInfos);
}

void ADokkaebiCharacter::UpdateCurseOrbPreviewVisuals_Implementation(bool bReady, bool bInstantHide)
{
	if (!CurseOrbPreviewMesh)
	{
		return;
	}

	if (bReady)
	{
		CurseOrbPreviewMesh->SetHiddenInGame(false);
		CurseOrbPreviewMesh->SetRelativeScale3D(FVector::OneVector);
		return;
	}

	if (bInstantHide)
	{
		CurseOrbPreviewMesh->SetRelativeScale3D(FVector::OneVector);
		CurseOrbPreviewMesh->SetHiddenInGame(true);
	}
}

void ADokkaebiCharacter::PlayPrepareCurseMontage()
{
	if (!PrepareCurseMontage)
	{
		return;
	}
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	UAnimInstance* Anim = MeshComp->GetAnimInstance();
	if (!Anim)
	{
		return;
	}
	// 같은 몽타주가 이미 돌면 중복 재생 방지 (선택)
	if (Anim->Montage_IsPlaying(PrepareCurseMontage))
	{
		return;
	}
	Anim->Montage_Play(PrepareCurseMontage);
}

void ADokkaebiCharacter::PlayFireCurseMontage()
{
	if (!FireCurseMontage || !GetMesh()) return;
	
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	// 준비 몽타주 끊고 발사로 넘어가는 경우가 많음
	if (PrepareCurseMontage && Anim->Montage_IsPlaying(PrepareCurseMontage))
	{
		Anim->Montage_Stop(0.1f, PrepareCurseMontage);
	}
	SetCurseFireMovementLock(true);
	GetWorldTimerManager().ClearTimer(CurseFireMoveUnlockTimerHandle);
	GetWorldTimerManager().SetTimer(
		CurseFireMoveUnlockTimerHandle,
		[this]()
		{
			SetCurseFireMovementLock(false);
		},
		FMath::Max(0.f, CurseFireMoveLockDuration),
		false
	);
	Anim->Montage_Play(FireCurseMontage);
}

void ADokkaebiCharacter::StopPrepareCurseMontage()
{
	if (!PrepareCurseMontage)
	{
		return;
	}

	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (Anim->Montage_IsPlaying(PrepareCurseMontage))
		{
			Anim->Montage_Stop(0.2f, PrepareCurseMontage);
		}
	}
}

void ADokkaebiCharacter::SetCurseFireMovementLock(bool bLocked)
{
	if (bLocked)
	{
		bCanMove = false;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}
		bCurseFireMovementLocked = true;
		return;
	}

	if (!bCurseFireMovementLocked)
	{
		return;
	}

	bCanMove = true;
	bCurseFireMovementLocked = false;
}

void ADokkaebiCharacter::OnRep_CurseOrbReady()
{
	ApplyCurseOrbReadyVisuals();
}

void ADokkaebiCharacter::ApplyCurseOrbReadyVisuals()
{
	UpdateCurseOrbPreviewVisuals(bCurseOrbReady, bCurseOrbHideInstant);
}

void ADokkaebiCharacter::SetCurseOrbReadyOnServer(bool bReady, bool bInstantHideWhenOff)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bReady)
	{
		if (bCurseOrbReady)
		{
			return;
		}
		bCurseOrbHideInstant = false;
		bCurseOrbReady = true;
	}
	else
	{
		if (!bCurseOrbReady)
		{
			return;
		}
		bCurseOrbHideInstant = bInstantHideWhenOff;
		bCurseOrbReady = false;
	}

	ApplyCurseOrbReadyVisuals();
}


void ADokkaebiCharacter::MulticastPlayDecoyVanishFX_Implementation(FVector Location)
{
	if (!DecoyVanishFX || !GetWorld())
	{
		return;
	}
	UParticleSystemComponent* FX = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		DecoyVanishFX,
		Location,
		FRotator::ZeroRotator,
		DecoyVanishFXScale,
		true);

	if (FX)
	{
		FX->CustomTimeDilation = DecoyVanishFXTimeDilation;
		FX->bAutoDestroy = true;

		const float KillAfter = DecoyVanishFXMaxLifetime / FMath::Max(DecoyVanishFXTimeDilation, 0.1f);
		TWeakObjectPtr<UParticleSystemComponent> WeakFX(FX);
		FTimerHandle KillFXHandle;
		GetWorld()->GetTimerManager().SetTimer(
			KillFXHandle,
			FTimerDelegate::CreateWeakLambda(FX, [WeakFX]()
			{
				if (UParticleSystemComponent* Comp = WeakFX.Get())
				{
					Comp->DeactivateSystem();
					Comp->DestroyComponent();
				}
			}),
			KillAfter,
			false);
	}
}