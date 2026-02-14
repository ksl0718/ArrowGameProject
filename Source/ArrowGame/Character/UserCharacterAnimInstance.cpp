// Fill out your copyright notice in the Description page of Project Settings.


#include "UserCharacterAnimInstance.h"
#include "../Character/UserCharacter.h"
#include "../Character/ArrowCharacter.h"
#include "../Weapon/Weapon.h"
#include "../Weapon/Bow.h"
#include "Kismet/KismetMathLibrary.h" // 방향 계산용 수학 함수
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UUserCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    APawn* Pawn = TryGetPawnOwner(); 
    if (!Pawn) return;

    AUserCharacter* Character = Cast<AUserCharacter>(Pawn);
    if (!Character) return;
    
    bIsDead = Character->IsDead();
    FVector Velocity = Character->GetVelocity();
    GroundSpeed = Velocity.Size2D();
    
    FRotator ActorRotation = Character->GetActorRotation();
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRotation);

    
    float TargetPitch = 0.f;
    // 1. 내 캐릭터 (Local): 반응 속도를 위해 즉시 반영되는 내 컨트롤러 값 사용
    if (Character->IsLocallyControlled())
    {
        TargetPitch = Character->GetControlRotation().Pitch;
        TargetPitch = FRotator::NormalizeAxis(TargetPitch);
    }
    // 2. 남의 캐릭터 (Remote/Server): 서버가 보내준 SyncPitch 사용
    else
    {
        // 복잡한 계산 X, 압축 해제 X -> 그냥 가져다 쓰면 됨
        TargetPitch = Character->GetSyncPitch();
    }

    // 3. [선택] 만약 에임 오프셋 방향이 반대라면 여기서만 뒤집으면 됨 (* -1)
    // TargetPitch *= -1.0f; 

    // 4. 각도 제한
    TargetPitch = FMath::Clamp(TargetPitch, -90.0f, 90.0f);

    // 5. 보간 (부드럽게)
    float InterpSpeed = Character->IsLocallyControlled() ? 0.0f : 15.0f;
    
    FRotator CurrentRot = FRotator(Pitch, 0, 0);
    FRotator GoalRot = FRotator(TargetPitch, 0, 0);
    
    if (InterpSpeed <= 0.f) Pitch = TargetPitch;
    else Pitch = FMath::RInterpTo(CurrentRot, GoalRot, DeltaSeconds, InterpSpeed).Pitch;
    
    if (Character->GetEquippedWeapon()) 
    {
        ABow* Bow = Cast<ABow>(Character->GetEquippedWeapon());
        if (Bow)
        {
            // 활의 상태를 복사 (동기화)
            bIsAiming = Bow->IsAiming(); 
            bIsCharging = Bow->IsCharging();
            //FString Role = Character->HasAuthority() ? TEXT("Server") : TEXT("Client");
            //UE_LOG(LogTemp, Warning, TEXT("[%s] Bow Found! Aiming: %s"), *Role, Bow->IsAiming() ? TEXT("TRUE") : TEXT("FALSE"));
        }
    }
    else
    {
        // 무기가 없으면 조준 해제
        bIsAiming = false;
        bIsCharging = false;
        //FString Role = Character->HasAuthority() ? TEXT("Server") : TEXT("Client");
        //UE_LOG(LogTemp, Error, TEXT("[%s] Weapon is NULL!"), *Role);
    }
    
    if (Character->GetCharacterMovement())
    {
        // 1. 현재 가속도(키보드 입력 등) 가져오기
        FVector Acceleration = Character->GetCharacterMovement()->GetCurrentAcceleration();
        
        // 2. 가속도가 0이 아닌지 체크 (키를 눌렀는가?)
        bool bHasInput = !Acceleration.IsNearlyZero();

        // 3. 로직 구현: (속도가 3보다 크고) AND (입력이 있을 때) -> 움직이는 것
        if (GroundSpeed > 3.0f && bHasInput)
        {
            bShouldMove = true;
        }
        else
        {
            bShouldMove = false;
        }
    }
}

void UUserCharacterAnimInstance::SetCanMove(bool bNewCanMove)
{
    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        AUserCharacter* Character = Cast<AUserCharacter>(Pawn);
        if (Character)
        {
            Character->bCanMove = bNewCanMove;
        }
    }
}
