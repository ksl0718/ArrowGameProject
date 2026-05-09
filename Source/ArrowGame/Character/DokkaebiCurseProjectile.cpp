#include "DokkaebiCurseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ArrowGame/Character/UserArcherCharacter.h"
#include "ArrowGame/Character/DokkaebiCharacter.h"

ADokkaebiCurseProjectile::ADokkaebiCurseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->InitSphereRadius(16.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Block+Hit만 쓰면 고속 투사체가 서버에서 원격 궁수 캡슐을 '스윕으로 스킵'하는 경우가 있어
	// Pawn은 Overlap으로 잡고 BeginOverlap에서 저주를 건다.
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ADokkaebiCurseProjectile::OnPawnOverlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1800.f;
	ProjectileMovement->MaxSpeed = 1800.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ADokkaebiCurseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSeconds);
}

void ADokkaebiCurseProjectile::SetCurseCaster(ADokkaebiCharacter* InCaster)
{
	CurseCaster = InCaster;
}

void ADokkaebiCurseProjectile::OnPawnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryApplyCurseToVictim(OtherActor);
}

void ADokkaebiCurseProjectile::TryApplyCurseToVictim(AActor* Candidate)
{
	if (!HasAuthority() || bCurseApplied || !IsValid(Candidate) || Candidate == this)
	{
		return;
	}
	if (Candidate == GetInstigator())
	{
		return;
	}

	AUserArcherCharacter* Victim = Cast<AUserArcherCharacter>(Candidate);
	if (!Victim && IsValid(Candidate->GetOwner()))
	{
		Victim = Cast<AUserArcherCharacter>(Candidate->GetOwner());
	}
	if (!Victim)
	{
		return;
	}

	ADokkaebiCharacter* Dokkaebi = CurseCaster.Get();
	if (!Dokkaebi)
	{
		Dokkaebi = Cast<ADokkaebiCharacter>(GetInstigator());
	}
	if (!Dokkaebi)
	{
		Dokkaebi = Cast<ADokkaebiCharacter>(GetOwner());
	}
	if (!IsValid(Dokkaebi))
	{
		UE_LOG(LogTemp, Error, TEXT("[CurseProjectile] No valid Dokkaebi caster; curse skipped"));
		bCurseApplied = true;
		Destroy();
		return;
	}

	bCurseApplied = true;
	Victim->ApplyCurseControl(CurseDuration, Dokkaebi);
	UE_LOG(LogTemp, Warning, TEXT("[CurseProjectile] Overlap Archer -> ApplyCurseControl"));
	Destroy();
}