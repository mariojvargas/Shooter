// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    /** Particles to spawn when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* ImpactParticles{ nullptr };

    /** Sound to play when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USoundCue* ImpactSound{ nullptr };

    /** Current health of this enemy */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float Health;

    /** Current health of this enemy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float MaxHealth;

    /** Name of the head bone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FString HeadBone;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // NOTE: because BulletHit is already decorated with UFUNCTION() we don't need to do it again here
    virtual void BulletHit_Implementation(FHitResult HitResult) override;

    virtual float TakeDamage(
        float DamageAmount, 
        struct FDamageEvent const& DamageEvent, 
        AController* EventInstigator, 
        AActor* DamageCauser) override;

    FORCEINLINE FString GetHeadBone() const { return HeadBone; }
};
