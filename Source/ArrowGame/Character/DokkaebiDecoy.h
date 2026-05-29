#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DokkaebiDecoy.generated.h"

UCLASS()
class ARROWGAME_API ADokkaebiDecoy : public ACharacter
{
	GENERATED_BODY()
public:
	ADokkaebiDecoy();

	/** 스폰 주체 MaxWalkSpeed에 곱함 (1 = 본체와 동일) */
	UPROPERTY(EditAnywhere, Category = "Decoy|Movement")
	float SpeedMultiplier = 1.f;

	/** 소유자 MaxStepHeight에 더해 미세 턱/바닥 이음을 넘기기 쉽게 함 */
	UPROPERTY(EditAnywhere, Category = "Decoy|Movement")
	float ExtraStepHeight = 12.f;

	/** 스폰 직후·BeginPlay에서 본체 이동 수치를 복사 */
	void SyncMovementFromOwner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	virtual void Tick(float DeltaTime) override;
};
