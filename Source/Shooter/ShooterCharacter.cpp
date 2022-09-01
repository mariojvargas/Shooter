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
#include "Components/WidgetComponent.h"
#include "Item.h"
#include "Weapon.h"

// Sets default values
AShooterCharacter::AShooterCharacter() : 
	BaseTurnRate(DEFAULT_BASE_TURN_RATE),
	BaseLookUpRate(DEFAULT_BASE_LOOK_UP_RATE),
	
	// Turn rates when aiming or not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	
	// Mouse sensitivity scale factors
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.35f),
	MouseAimingLookUpRate(0.35f),

	bAiming(false),
	
	// Camera field of view values
	CameraDefaultFieldOfView(0.f),
	CameraZoomedFieldOfView(AIMING_ZOOM_FOV),
	CameraCurrentFieldOfView(0.f),
	ZoomInterpolationSpeed(CAMERA_ZOOM_INTERPOLATION_SPEED),

	// Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),

	// Bullet fire timer
	ShootTimeDurationSeconds(0.05f),
	bFiringBullet(false),

	// Automatic gunfire configuration
	// NOTE: Fire rate must be higher than crosshair 
	//       interpolation speed (ShootTimeDurationSeconds)
	bFireButtonPressed(false),
	bShouldFire(true),
	AutomaticFireRate(0.1f),

	// Item tracing
	bShouldTraceForOverlappingItems(false),

	// Camera interp location
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),

	// Ammo
	Starting9mmAmmoAmount(85),
	StartingARAmmoAmount(120)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera spring arm, which pulls in towards character if there is a collision 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CAMERA_BOOM_ARM_LENGTH;	// Follow character at this distance
	CameraBoom->bUsePawnControlRotation = true;	// Rotate the spring arm based on the character controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	// Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false;	// Camera does not rotate relative to spring arm

	// Don't rotate when the controller rotates
	// because Controller should only affect camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	// we should rotate along with the controller
	bUseControllerRotationYaw = true;

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
	
	if (FollowCamera)
	{
		CameraDefaultFieldOfView = GetFollowCamera()->FieldOfView;
		CameraCurrentFieldOfView = CameraDefaultFieldOfView;
	}

	EquipWeapon(SpawnDefaultWeapon());

	InitializeAmmoMap();
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	InterpolateAimCameraZoom(DeltaTime);

	RefreshAimingOrHipLookRates();

	CalculateCrosshairSpread(DeltaTime);

	TraceForOverlappingItems();
}


void AShooterCharacter::InterpolateAimCameraZoom(float DeltaTime)
{
	if (IsAiming())
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(
			CameraCurrentFieldOfView, 
			CameraZoomedFieldOfView, 
			DeltaTime, 
			ZoomInterpolationSpeed);
	}
	else
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(
			CameraCurrentFieldOfView, 
			CameraDefaultFieldOfView, 
			DeltaTime, 
			ZoomInterpolationSpeed);
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFieldOfView);
}

void AShooterCharacter::RefreshAimingOrHipLookRates()
{
	if (IsAiming())
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
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

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::TurnWithMouse);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUpWithMouse);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("SelectButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("SelectButton", EInputEvent::IE_Released, this, &AShooterCharacter::SelectButtonReleased);
}

void AShooterCharacter::TurnWithMouse(float Value)
{
	const float MouseTurnSensitivity = IsAiming() ? MouseAimingTurnRate : MouseHipTurnRate;

	AddControllerYawInput(Value * MouseTurnSensitivity);
}

void AShooterCharacter::LookUpWithMouse(float Value)
{
	const float MouseLookUpSensitivity = IsAiming() ? MouseAimingLookUpRate : MouseHipLookUpRate;
	
	AddControllerPitchInput(Value * MouseLookUpSensitivity);
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

		FVector BeamEndLocation;
		if (GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndLocation))
		{
			// Spawn impact particles after weapon beam end point is updated
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), 
					ImpactParticles, 
					BeamEndLocation);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(), 
				BeamParticles, 
				SocketTransform);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndLocation);
			}
		}
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	BeginCrosshairBulletFire();
}


void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		// TODO: Maybe an event is better?
		TraceHitItem->StartItemCurve(this);
	}
}

void AShooterCharacter::SelectButtonReleased()
{

}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation)
{
	FHitResult CrosshairHitResult;

	if (TryGetTraceUnderCrosshairs(CrosshairHitResult, OutBeamEndLocation))
	{
		OutBeamEndLocation = CrosshairHitResult.Location;
	}

	// Peform a second trace from gun barrel in case there are 
	// objects between gun barrel and impact point
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	// Extend second line trace farther by 25%, 
	// taking into account last beam end location and start of muzzle socket
	const FVector StartToEnd{ OutBeamEndLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };	

	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit, 
		WeaponTraceStart, 
		WeaponTraceEnd, 
		ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamEndLocation = WeaponTraceHit.Location;
	}

	return WeaponTraceHit.bBlockingHit;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}


void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, 
		VelocityMultiplierRange, 
		Velocity.Size());

	if (GetCharacterMovement()->IsFalling())
	{
		// Spread crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor, 
			2.5f, 
			DeltaTime, 
			2.25f);
	}
	else
	{
		// Shrink crosshairs rapidly once on ground
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor, 
			0.f, 
			DeltaTime, 
			30.f);
	}

	if (IsAiming())
	{
		// Crosshairs should shrink in quickly by a small amount
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f);
	}
	else
	{
		// Crosshairs should quickly return to normal when we're not aiming
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			30.f);
	}

	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.3f,
			DeltaTime,
			60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.f,
			DeltaTime,
			60.f);
	}

	CrosshairSpreadMultiplier = 0.5f 
		+ CrosshairVelocityFactor 
		+ CrosshairInAirFactor
		+ CrosshairShootingFactor
		- CrosshairAimFactor;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::BeginCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer, 
		this, 
		&AShooterCharacter::EndCrosshairBulletFire, 
		ShootTimeDurationSeconds);
}

void AShooterCharacter::EndCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;

		GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&AShooterCharacter::ResetAutoFire,
			AutomaticFireRate
		);
	}
}

void AShooterCharacter::ResetAutoFire()
{
	bShouldFire = true;

	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TryGetTraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutTraceEndOrHitLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

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
		// trace crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * MAX_LINE_TRACE_DISTANCE };

		OutTraceEndOrHitLocation = End;
		
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult, 
			Start, 
			End, 
			ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutTraceEndOrHitLocation = OutHitResult.Location;

			return true;
		}
	}

	return false;
}

void AShooterCharacter::AddOverlappedItemCount(int32 Amount)
{
	// TODO: probably should extract bShouldTraceForItems from here but copied as is from course
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForOverlappingItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForOverlappingItems = true;
	}
}

void AShooterCharacter::TraceForOverlappingItems()
{
	if (!bShouldTraceForOverlappingItems)
	{
		if (LastTracedPickupItem)
		{
			LastTracedPickupItem->GetPickupWidget()->SetVisibility(false);
			LastTracedPickupItem = nullptr;
		}

		return;
	}

	FHitResult ItemTraceResult;
	FVector IgnoredTraceEndOrHitLocation;
	if (!TryGetTraceUnderCrosshairs(ItemTraceResult, IgnoredTraceEndOrHitLocation))
	{
		return;
	}

	// TODO: Refactor into maybe HitItem->ShowStatsHud()/HideStatsHud() 
	TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
	if (TraceHitItem && TraceHitItem->GetPickupWidget())
	{
		TraceHitItem->GetPickupWidget()->SetVisibility(true);
	}

	if (LastTracedPickupItem && LastTracedPickupItem != TraceHitItem)
	{
		LastTracedPickupItem->GetPickupWidget()->SetVisibility(false);
	}

	LastTracedPickupItem = TraceHitItem;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip)
	{
		return;
	}

	const USkeletalMeshSocket* HandWeaponSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandWeaponSocket)
	{
		HandWeaponSocket->AttachActor(WeaponToEquip, GetMesh());
	}

	EquippedWeapon = WeaponToEquip;

	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();

		EquippedWeapon = nullptr;
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	LastTracedPickupItem = nullptr;
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	// Desired = cameraWorldLocation + forward * A + Up * B;
	return CameraWorldLocation 
		+ CameraForward * CameraInterpDistance
		+ FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::LoadPickupItem(AItem* Item)
{
	AWeapon* Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmoAmount);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmoAmount);
}