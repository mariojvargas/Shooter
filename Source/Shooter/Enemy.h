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

    UFUNCTION(BlueprintNativeEvent)
    void ShowHealthBar();

    void ShowHealthBar_Implementation();

    UFUNCTION(BlueprintImplementableEvent)
    void HideHealthBar();

    void Die();

    void PlayHitMontage(FName SectionName, float PlayRate = 1.f);

    void ResetHitReactTimer();

    UFUNCTION(BlueprintCallable)
    void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

    UFUNCTION()
    void DestroyHitNumber(UUserWidget* HitNumber);

    void UpdateHitNumbers();

    void AgroSphereOverlap();

    /** Called when something overlaps with the enemy agro sphere */
    UFUNCTION()
	void AgroSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable)
    void SetStunned(bool Value);

    UFUNCTION()
    void CombatRangeOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult);

    UFUNCTION()
    void OnCombatRangeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable)
    void PlayAttackMontage(FName SectionName, float PlayRate);

    UFUNCTION(BlueprintPure)
    FName GetAttackSectionName() const;

    UFUNCTION()
    void OnLeftWeaponOverlap(
        UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, 
        UPrimitiveComponent* OtherComponent, 
        int32 OtherBodyIndex, 
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnRightWeaponOverlap(
        UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable)
    void ActivateLeftWeapon();

    UFUNCTION(BlueprintCallable)
    void DeactivateLeftWeapon();
    
    UFUNCTION(BlueprintCallable)
    void ActivateRightWeapon();
    
    UFUNCTION(BlueprintCallable)
    void DeactivateRightWeapon();

    void DoDamage(class AShooterCharacter* Victim);

    void SpawnBlood(AShooterCharacter* Victim, FName SocketName);

private:
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

    /** Time to display health bar once shot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HealthBarDisplayTime;

    FTimerHandle HealthBarTimer;

    /** Montage containing hit and death animations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HitMontage{ nullptr };

    FTimerHandle HitReactTimer;

    bool bCanHitReact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMax;

    /** Map to store HitNumber widgets and their hit locations */
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    TMap<UUserWidget*, FVector> HitNumbers;

    /** Time before a HitNumber is removed from the screen */
    UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitNumberDestroyTime;

    /** Behavior tree for the AI character */
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    class UBehaviorTree* BehaviorTree{ nullptr };

    /** Point for the enemy to move to */
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint;

    /** Second point for the enemy to move to */
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint2;

    class AEnemyController* EnemyController{ nullptr };

    /** Overlap sphere when the enemy becomes hostile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* AgroSphere{ nullptr };

    /** true when playing the "get hit" animation */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bStunned;

    /** Chance of being stunned. 0: no stun chance, 1: 100% stun chance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float StunChance;

    /** true when in attack range. Enemy will attack */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bInAttackRange;

    /** Sphere for attack range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USphereComponent* CombatRangeSphere{ nullptr };

    /** Montage containing different attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* AttackMontage{ nullptr };

    FName AttackLFast;
    FName AttackRFast;
    FName AttackL;
    FName AttackR;

    /** Collision volume for the left weapon */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* LeftWeaponCollision{ nullptr };

    /** Collision volume for the right weapon */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UBoxComponent* RightWeaponCollision{ nullptr };

    /** Base damage for this enemy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName LeftWeaponSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName RightWeaponSocket;

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

    UFUNCTION(BlueprintImplementableEvent)
    void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadshot);

    FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
};
