// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon() :
    ThrowWeaponTime(DEFAULT_THROW_WEAPON_TIME),
    bFalling(false),
    Ammo(0)
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Ensure weapon is upright
    if (IsFalling())
    {
        EnsureWeaponIsUpright();
    }
}

void AWeapon::ThrowWeapon()
{
   EnsureWeaponIsUpright();

    const FVector MeshForewardVector{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRightVector{ GetItemMesh()->GetRightVector() };

    // Direction in which character throws weapon
    FVector ImpulseDirection = MeshRightVector.RotateAngleAxis(-20.f, MeshForewardVector);

    float RandomRotation{ FMath::FRandRange(0, 30.f) };
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    ImpulseDirection *= 10'000.f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;

    GetWorldTimerManager().SetTimer(
        ThrowWeaponTimer, 
        this, 
        &AWeapon::StopFalling, 
        ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
    bFalling = false;

    SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::EnsureWeaponIsUpright()
{
    const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AWeapon::DecrementAmmo()
{
    if (Ammo - 1 <= 0)
    {
        Ammo = 0;
    }
    else
    {
        Ammo--;
    }
}

bool AWeapon::TryGetBarrelSocketTransform(FTransform& OutBarrelSocketTransform) const
{
    if (GetItemMesh())
    {
        const USkeletalMeshSocket* BarrelSocket = GetItemMesh()->GetSocketByName("BarrelSocket");
        if (BarrelSocket)
        {
            OutBarrelSocketTransform = BarrelSocket->GetSocketTransform(GetItemMesh());

            return true;
        }
    }

    return false;
}