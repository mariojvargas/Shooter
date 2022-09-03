// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
    Speed(0.f),
    bIsInAir(false),
    bIsAccelerating(false),
    MovementOffsetYaw(0.f),
    LastMovementOffsetYaw(0.f),
    bAiming(false),
    CharacterYaw(0.f),
    CharacterYawLastFrame(0.f),
    RootYawOffset(0.f),
    RotationCurveValue(0.f),
    RotationCurveValueLastFrame(0.f)
{
}

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

    TurnInPlace();
}

void UShooterAnimInstance::TurnInPlace()
{
    if (!ShooterCharacter)
    {
        return;
    }

    if (Speed > 0)
    {
        // Don't turn in place because character is moving
        RootYawOffset = 0.f;
        CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        CharacterYawLastFrame = CharacterYaw;
        RotationCurveValueLastFrame = 0.f;
        RotationCurveValue = 0.f;
    }
    else
    {
        CharacterYawLastFrame = CharacterYaw;
        CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

        const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

        // Root yaw offset updated and clamped [-180, 180]
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

        const float TurningCurveValue{ GetCurveValue(TEXT("Turning")) };

        // 1.0 if turning; 0 if not
        if (TurningCurveValue > 0)
        {
            RotationCurveValueLastFrame = RotationCurveValue;
            RotationCurveValue = GetCurveValue(TEXT("Rotation"));

            const float DeltaRotation{ RotationCurveValue - RotationCurveValueLastFrame };

            // If root yaw offset is positive, we're turning left
            // but if root yaw offset is negative, we're turning right
            RootYawOffset = (RootYawOffset > 0) 
                ? RootYawOffset - DeltaRotation 
                : RootYawOffset + DeltaRotation;

            const float AbsoluteRootYawOffset = FMath::Abs(RootYawOffset);
            if (AbsoluteRootYawOffset > 90.f)
            {
                const float ExcessYaw{ AbsoluteRootYawOffset - 90.f };
                RootYawOffset = (RootYawOffset > 0) 
                    ? RootYawOffset - ExcessYaw 
                    : RootYawOffset + ExcessYaw;
            }
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                1, 
                -1,
                FColor::Green, 
                FString::Printf(TEXT("CharacterYaw: %f"), CharacterYaw)
            );

            GEngine->AddOnScreenDebugMessage(
                2, 
                -1,
                FColor::Yellow, 
                FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset)
            );
        }
    }
    
}
