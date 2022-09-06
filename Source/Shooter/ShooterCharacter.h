// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

class AWeapon;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Ready	 UMETA(DisplayName = "Ready"),
	ECS_FiringInProgress UMETA(DisplayName = "FiringInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

private:
	const float DEFAULT_BASE_TURN_RATE = 45.f;
	const float DEFAULT_BASE_LOOK_UP_RATE = 45.f;
	const float AIMING_ZOOM_FOV = 35.f;
	const float CAMERA_BOOM_ARM_LENGTH = 180.f;
	const float CAMERA_ZOOM_INTERPOLATION_SPEED = 20.f;
	const float MAX_LINE_TRACE_DISTANCE = 50000.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override;

	/** Moves character forwards or backwards */
	void MoveForward(float Value);

	/** Moves character to the right or left */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn character at a given rate.
	 * @param Rate	Normalized rate (i.e. 1.0 means 100% of desired turn rate).
	*/
	void TurnAtRate(float Rate);

	/**
	 * Called via input to look up or down at a given rate.
	 * @param Rate	Normalized rate (i.e. 1.0 means 100% of desired rate).
	 */
	void LookUpAtRate(float Rate);

	void TurnWithMouse(float Value);

	void LookUpWithMouse(float Value);


	/**
	 * Called when Fire Button is pressed.
	 */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation);

	void AimingButtonPressed();

	void AimingButtonReleased();

	void InterpolateAimCameraZoom(float DeltaTime);

	void RefreshAimingOrHipLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void ResetAutoFire();

	bool TryGetTraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutTraceEndOrHitLocation);

	void TraceForOverlappingItems();

	AWeapon* SpawnDefaultWeapon();

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void DropWeapon();

	void SelectButtonPressed();

	void SelectButtonReleased();

	void SwapWeapon(AWeapon* WeaponToSwap);

	void InitializeAmmoMap();

	bool WeaponHasAmmo() const;

	void PlayFireSound();

	void SendBullet();

	void PlayGunFireMontage();

	void ReloadButtonPressed();

	void ReloadWeapon();

	/** Checks whether the character has ammo of the equipped weapon's ammo type */
	bool IsCarryingAmmo();

	UFUNCTION(BlueprintCallable)
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	/** Interps capsule half-height when crouching or standing */
	void InterpCapsuleHalfHeight(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera{ nullptr };

	/** Base turn rate in degrees per second. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look-up/down rate in degrees per second. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate while not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate when not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Mouse sensitivity turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.1", ClampMax = "1.0",  UIMin = "0.1", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Mouse sensitivity look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.1", ClampMax = "1.0",  UIMin = "0.1", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Mouse sensitivity turn rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.1", ClampMax = "1.0",  UIMin = "0.1", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** Mouse sensitivity look up rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.1", ClampMax = "1.0",  UIMin = "0.1", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/** Gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound{ nullptr };

	/** Firing flash spawned at BarrelSocket of weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash{ nullptr };

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles{ nullptr };

	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles{ nullptr };

	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float CameraDefaultFieldOfView;

	float CameraZoomedFieldOfView;

	float CameraCurrentFieldOfView;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpolationSpeed;

	/** Determines the spread of the crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/** Velocity component for crosshair's spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In-air component for crosshair's spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/** Aim component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;


	/** Left mouse button or gamepad right-trigger pressed */
	bool bFireButtonPressed;

	float AutomaticFireRate;

	/** Timer between gunshots */
	FTimerHandle AutoFireTimer;

	/** Allows tracing for items every frame when set to true */
	bool bShouldTraceForOverlappingItems;

	int32 OverlappedItemCount;	// Can be int8

	/** The last AItem traced in most recent frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* LastTracedPickupItem{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/** Current item hit by trace or null if no item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	/** Combat state can only occur when not occupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	/** Montage for reload animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadAnimMontage{ nullptr };

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/** Distance outward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	/** Distance upward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmoAmount;

	/** Starting assault rifle ammo amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmoAmount;

	/** Transform of the gun's magazine clip when it's grabbed during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	/** Scene component to attach to the character's hand during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** Regular movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	float CurrentCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	/** Ground friction while standing (not crouching) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const 
	{ 
		return CameraBoom; 
	}

	FORCEINLINE UCameraComponent* GetFollowCamera() const 
	{ 
		return FollowCamera; 
	}

	FORCEINLINE bool IsAiming() const
	{
		return bAiming;
	}

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int32 GetOverlappedItemCount() const
	{
		return OverlappedItemCount;
	}

	FORCEINLINE ECombatState GetCombatState() const
	{
		return CombatState;
	}

	FORCEINLINE bool IsCrouching() const
	{ 
		return bCrouching; 
	}

	/** Adds given amount to current number of overlapped items */
	void AddOverlappedItemCount(int32 Amount);

	FVector GetCameraInterpLocation();

	void LoadPickupItem(AItem* Item);
};
