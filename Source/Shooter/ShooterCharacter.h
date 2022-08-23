// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

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

	/**
	 * Called when Fire Button is pressed.
	 */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation);

	void AimingButtonPressed();

	void AimingButtonReleased();

	void InterpolateAimCameraZoom(float DeltaTime);

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
};
