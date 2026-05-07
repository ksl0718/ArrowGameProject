#include "DokkaebiCurseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ArrowGame/Character/UserArcherCharacter.h"

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
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->OnComponentHit.AddDynamic(this, &ADokkaebiCurseProjectile::OnHit);
	
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

void ADokkaebiCurseProjectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}
	if (AUserArcherCharacter* Victim = Cast<AUserArcherCharacter>(OtherActor))
	{
		Victim->ApplyCurseControl(CurseDuration);
		UE_LOG(LogTemp, Warning, TEXT("[CurseProjectile] Hit Archer -> ApplyCurseControl"));
	}
	Destroy();
}