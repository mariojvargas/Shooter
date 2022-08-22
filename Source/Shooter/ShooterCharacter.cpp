// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

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
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	// Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false;	// Camera does not rotate relative to spring arm

	// Don't rotate when the controller rotates
	// because Controller should only affect camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;	// true => character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);	// ... at the rotation rate here
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
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

	// Bind character turn to Mouse horizontal direction
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	// Bind character look up/down to Mouse vertical direction
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);
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

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		// Get current size of viewport
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		// Get screen space location of crosshair
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		CrosshairLocation.Y -= 50.f;

		// Get world position and direction of crosshair
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		bool bScreenToWorldSuccess = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);

		if (bScreenToWorldSuccess)
		{
			const FVector LineTraceStart{ CrosshairWorldPosition };
			const FVector LineTraceEnd{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

			FVector BeamEndPoint{ LineTraceEnd };

			// Trace outwards from crosshair world location
			FHitResult ScreenTraceHit;
			GetWorld()->LineTraceSingleByChannel(
				ScreenTraceHit,
				LineTraceStart, 
				LineTraceEnd, 
				ECollisionChannel::ECC_Visibility);

			if (ScreenTraceHit.bBlockingHit)
			{
				BeamEndPoint = ScreenTraceHit.Location;

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ScreenTraceHit.Location);
				}

				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(), 
						BeamParticles, 
						SocketTransform);

					if (Beam)
					{
						Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
					}
				}
			}
		}

		// const FVector LineTraceStart{ SocketTransform.GetLocation() };
		// const FQuat Rotation{ SocketTransform.GetRotation() };
		// const FVector RotationAxis{ Rotation.GetAxisX() };
		// const FVector LineTraceEnd{ LineTraceStart + RotationAxis * 50'000.f };

		// FVector BeamEndPoint{ LineTraceEnd };

		// FHitResult FireHit;
		// GetWorld()->LineTraceSingleByChannel(FireHit, LineTraceStart, LineTraceEnd, ECollisionChannel::ECC_Visibility);
		// if (FireHit.bBlockingHit)
		// {
		// 	DrawDebugLine(GetWorld(), LineTraceStart, LineTraceEnd, FColor::Red, false, 2.f);
		// 	DrawDebugPoint(GetWorld(), FireHit.Location, 5, FColor::Blue, false, 2.f);

		// 	BeamEndPoint = FireHit.Location;

		// 	if (ImpactParticles)
		// 	{
		// 		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
		// 	}
		// }

		// if (BeamParticles)
		// {
		// 	UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		// 	Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
		// }
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}


}
