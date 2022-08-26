// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

class AWeapon;

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

	void BeginCrosshairBulletFire();

	UFUNCTION()
	void EndCrosshairBulletFire();

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

	/** Firing flash spawned at BarrelSocket of character skeletal mesh */
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

	float ShootTimeDurationSeconds;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	/** Left mouse button or gamepad right-trigger pressed */
	bool bFireButtonPressed;

	/** Determines whether we can fire. False when waiting */
	bool bShouldFire;

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

	/** Adds given amount to current number of overlapped items */
	void AddOverlappedItemCount(int32 Amount);
};
