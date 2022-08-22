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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound{ nullptr };

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const 
	{ 
		return CameraBoom; 
	}

	FORCEINLINE UCameraComponent* GetFollowCamera() const 
	{ 
		return FollowCamera; 
	}
};
