// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAmmoType AmmoType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeaponAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MagazineCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundCue* PickupSound{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundCue* EquipSound{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* ItemMesh{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* InventoryIcon{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* AmmoIcon{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* MaterialInstance{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaterialIndex;
};

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

	bool TryGetBarrelSocketTransform(FTransform& OutBarrelSocketTransform) const;

	bool IsClipFull() const;

protected:
	void StopFalling();

    virtual void OnConstruction(const FTransform& Transform) override;

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	void EnsureWeaponIsUpright();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32  MagazineCapacity;

	/** Ammo count for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	/** Animation montage section name for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSectionName;

	/** Indicates whether the clip is moving while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

    /** Data table for weapon properties */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UDataTable* WeaponDataTable{ nullptr };

    int32 PreviousMaterialIndex;

public:
	FORCEINLINE bool IsFalling() const
	{
		return GetItemState() == EItemState::EIS_Falling && bFalling;
	}

	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }

	void DecrementAmmo();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageSectionName() const { return ReloadMontageSectionName; }

	void ReloadAmmo(int32 Amount);

	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

	FORCEINLINE void SetMovingClip(bool bValue) { bMovingClip = bValue; }
};
