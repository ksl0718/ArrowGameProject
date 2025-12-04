// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "ArrowCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWeapon::OnEquip(AArrowCharacter* NewOwner)
{
    OwnerCharacter = NewOwner;
    SetOwner(NewOwner);
}

void AWeapon::OnUnequip()
{
    OwnerCharacter = nullptr;
}

