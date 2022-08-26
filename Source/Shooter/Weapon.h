// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

private:
	const float DEFAULT_THROW_WEAPON_TIME = 0.7f;

public:
	AWeapon();
	
	virtual void Tick(float DeltaTime);

	/** Tosses weapon with impulse */
	void ThrowWeapon();

protected:
	void StopFalling();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	void EnsureWeaponIsUpright();

public:
	FORCEINLINE bool IsFalling() const
	{
		return GetItemState() == EItemState::EIS_Falling && bFalling;
	}
};
