// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() : 
	BaseTurnRate(DEFAULT_BASE_TURN_RATE),
	BaseLookUpRate(DEFAULT_BASE_LOOK_UP_RATE)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera spring arm, which pulls in towards character if there is a collision 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;	// Follow character at this distance
	CameraBoom->bUsePawnControlRotation = true;	// Rotate the spring arm based on the character controller

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	// Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false;	// Camera does not rotate relative to spring arm
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller == nullptr || Value == 0)
	{
		return;
	}

	// TODO: This is too complicated, as it can simply be AddMovementInput(GetActorForwardVector(), Value);

	// Determine forward direction
	const FRotator Rotation{ Controller->GetControlRotation() };
	const FRotator YawRotation{0, Rotation.Yaw, 0};
	const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

	AddMovementInput(Direction, Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller == nullptr || Value == 0)
	{
		return;
	}

	// TODO: This is too complicated, as it can simply be AddMovementInput(GetActorRightVector(), Value);

	// Determine right-side direction
	const FRotator Rotation{ Controller->GetControlRotation() };
	const FRotator YawRotation{0, Rotation.Yaw, 0};
	const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

	AddMovementInput(Direction, Value);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Assertion: Use "check()" macro to halt execution if PlayerInputComponent is invalid
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// base turn rate (deg/sec)  * delta time (sec/frame)  => degrees per frame
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}