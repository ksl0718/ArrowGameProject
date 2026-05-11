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

// --- 복제: 은신 전 클라, 스킬 상태·투시 종료 시각은 시전자(Owner)만 ---

void ADokkaebiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADokkaebiCharacter, bIsStealthed);
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

void ADokkaebiCharacter::PlayHitReaction()
{
	if (!HitMontage || bIsDead) return;

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Play(HitMontage);
	}
}

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
			EnhancedInput->BindAction(CurseSkillAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::FireCurseProjectile);
		}
		
		if (SpiritSightAction)
		{
			EnhancedInput->BindAction(SpiritSightAction, ETriggerEvent::Started, this, &ADokkaebiCharacter::Input_SpiritSight);
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
	// 본인: 반투명+포스트 / 타인: 메시 숨김
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Stealth OnRep: bIsStealthed=%d, Local=%d"),
		bIsStealthed ? 1 : 0,
		IsLocallyControlled() ? 1 : 0);

		GetMesh()->SetScalarParameterValueOnMaterials(TEXT("Opacity"),bIsStealthed ? 0.3f : 1.0f);

		if (FollowCamera)
    	{
        FollowCamera->PostProcessBlendWeight = bIsStealthed ? 1.0f : 0.0f;
    	}
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

void ADokkaebiCharacter::FireCurseProjectile()
{
	const FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0,0,50.f);
	const FRotator SpawnRot = GetControlRotation();

	Server_FireCurseProjectile(SpawnLoc, SpawnRot); // 실제 스폰·쿨다운은 서버
}

void ADokkaebiCharacter::Server_FireCurseProjectile_Implementation(FVector SpawnLoc, FRotator SpawnRot)
{
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
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ADokkaebiCurseProjectile* Proj = GetWorld()->SpawnActor<ADokkaebiCurseProjectile>(
		CurseProjectileClass,
		SpawnLoc,
		FRotator(0.f, SpawnRot.Yaw, 0.f),
		Params
	);
	if (Proj)
	{
		Proj->SetCurseCaster(this);
		// (선택) 이미 Params로 넣었어도 한 번 더 명시해도 무방
		Proj->SetInstigator(this);
		Proj->SetOwner(this);
		UE_LOG(LogTemp, Warning, TEXT("[Dokkaebi] Curse projectile fired"));
	}
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
							M->SetRenderCustomDepth(false);
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

		if (ACharacter* OtherChar = Cast<ACharacter>(OtherPawn))
		{
			if (USkeletalMeshComponent* EnemyMesh = OtherChar->GetMesh())
			{
				if (!EnemyMesh->bRenderCustomDepth)
				{
					EnemyMesh->SetRenderCustomDepth(true);
					EnemyMesh->SetCustomDepthStencilValue(200); // 궁수 쪽(1)과 겹치지 않게
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
