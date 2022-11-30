// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "EnemyController.h"
#include "DrawDebugHelpers.h"

// Sets default values
AEnemy::AEnemy() :
    Health(100.f),
    MaxHealth(100.f),
    HealthBarDisplayTime(4.f),
    bCanHitReact(true),
    HitReactTimeMin(0.5f),
    HitReactTimeMax(3.f),
    HitNumberDestroyTime(1.5f),
    bStunned(false),
    StunChance(0.5f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
    AgroSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

    AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
    DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);

    const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
    DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Cyan, true);

    // Get the AI controller
    EnemyController = Cast<AEnemyController>(GetController());

    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);

        EnemyController->RunBehaviorTree(BehaviorTree);
    }
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }

    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), 
            ImpactParticles, 
            HitResult.Location, 
            FRotator(0.f), 
            true
        );
    }

    ShowHealthBar();

    const float Stunned = FMath::FRandRange(0.f, 1.f);
    if (Stunned <= StunChance)
    {
        // Stun the enemy
        PlayHitMontage(FName("HitReactFront"));
        bStunned = true;
    }
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (Health - DamageAmount <= 0.f)
    {
        Health = 0;
        Die();
    }
    else
    {
        Health -= DamageAmount;
    }

    return DamageAmount;
}

void AEnemy::ShowHealthBar_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealthBarTimer);
    GetWorldTimerManager().SetTimer(
        HealthBarTimer, 
        this, 
        &AEnemy::HideHealthBar, 
        HealthBarDisplayTime);
}

void AEnemy::Die()
{
    HideHealthBar();
}

void AEnemy::PlayHitMontage(FName SectionName, float PlayRate)
{
    if (!bCanHitReact)
    {
        return;
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(HitMontage, PlayRate);
        AnimInstance->Montage_JumpToSection(SectionName, HitMontage);
    }

    bCanHitReact = false;

    const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
    GetWorldTimerManager().SetTimer(
        HitReactTimer, 
        this, 
        &AEnemy::ResetHitReactTimer, 
        HitReactTime);
}

void AEnemy::ResetHitReactTimer()
{
    bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
    HitNumbers.Add(HitNumber, Location);

    FTimerDelegate HitNumberDelegate;
    HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);

    FTimerHandle HitNumberTimer;
    GetWorld()->GetTimerManager().SetTimer(
        HitNumberTimer, 
        HitNumberDelegate, 
        HitNumberDestroyTime, 
        false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
    HitNumbers.Remove(HitNumber);
    HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
    for (auto& HitPair : HitNumbers)
    {
        UUserWidget* HitNumber{ HitPair.Key };
        const FVector Location{ HitPair.Value };

        FVector2D ScreenPosition;
        UGameplayStatics::ProjectWorldToScreen(
            GetWorld()->GetFirstPlayerController(), 
            Location, 
            ScreenPosition);

        HitNumber->SetPositionInViewport(ScreenPosition);
    }
}

void AEnemy::AgroSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
    }
}
