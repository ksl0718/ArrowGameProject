// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class ARROWGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void OnEquip(class AArrowCharacter* NewOwner);

	UFUNCTION(BlueprintCallable)
	virtual void OnUnequip();

	virtual void StartAim() {}
	virtual void StopAim() {}
	virtual void StartDraw() {}
	virtual void EndDraw() {}

protected:

	UPROPERTY(BlueprintReadOnly)
	class AArrowCharacter* OwnerCharacter;

};
