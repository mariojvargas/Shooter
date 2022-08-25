// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (ShooterCharacter == nullptr)
    {
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    }

    if (ShooterCharacter)
    {
        // Get the speed of the character from velocity
        FVector Velocity{ ShooterCharacter->GetVelocity() };
        Velocity.Z = 0; // We just need the lateral velocity components, so Z isn't needed
        Speed = Velocity.Size();

        bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

        // Check whether character is moving
        bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

        FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

        if (ShooterCharacter->GetVelocity().Size() > 0.f)
        {
            LastMovementOffsetYaw = MovementOffsetYaw;
        }

        bAiming = ShooterCharacter->IsAiming();

        // if (GEngine)
        // {
        //     FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
        //     GEngine->AddOnScreenDebugMessage(1, 0, FColor::White, RotationMessage);

        //     FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
        //     GEngine->AddOnScreenDebugMessage(2, 0, FColor::Green, MovementRotationMessage);

        //     FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);
        //     GEngine->AddOnScreenDebugMessage(3, 0, FColor::Cyan, OffsetMessage);
        // }
    }
}
