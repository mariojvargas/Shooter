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
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/DamageType.h"
#include "Engine/SkeletalMeshSocket.h"

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
    StunChance(0.5f),
    AttackLFast(TEXT("AttackLFast")),
    AttackRFast(TEXT("AttackRFast")),
    AttackL(TEXT("AttackL")),
    AttackR(TEXT("AttackR")),
    BaseDamage(20.f),
    LeftWeaponSocket(TEXT("FX_Trail_L_01")),
    RightWeaponSocket(TEXT("FX_Trail_R_01"))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
    AgroSphere->SetupAttachment(GetRootComponent());

    CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
    CombatRangeSphere->SetupAttachment(GetRootComponent());

    LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
    LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponSocket"));

    RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
    RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponSocket"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

    AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);

    CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
    CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeEndOverlap);

    LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    // Ignore camera for mesh and capsule
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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
        SetStunned(true);
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

void AEnemy::SetStunned(bool Value)
{
    bStunned = Value;

    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Value);
    }
}

void AEnemy::CombatRangeOverlap(
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

    auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
    if (ShooterCharacter)
    {
        bInAttackRange = true;

        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
        }
    }

}

void AEnemy::OnCombatRangeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
    if (ShooterCharacter)
    {
        bInAttackRange = false;

        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
        }
    }
}

void AEnemy::PlayAttackMontage(FName SectionName, float PlayRate)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        AnimInstance->Montage_Play(AttackMontage, PlayRate);
        AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
    }
}

FName AEnemy::GetAttackSectionName() const
{
    const int32 SectionNumber{ FMath::RandRange(1, 4) };

    // TODO: this should be an array
    switch (SectionNumber)
    {
        case 1:
            return AttackLFast;

        case 2:
            return AttackRFast;

        case 3:
            return AttackL;

        default:
            return AttackR;
    }
}

void AEnemy::OnLeftWeaponOverlap(
        UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult)
{
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        SpawnBlood(Character, LeftWeaponSocket);
    }
}

void AEnemy::OnRightWeaponOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComponent, 
    int32 OtherBodyIndex, 
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        SpawnBlood(Character, RightWeaponSocket);
    }
}

void AEnemy::ActivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DoDamage(AShooterCharacter* Victim)
{
    if (Victim == nullptr)
    {
        return;
    }

    UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyController, this, UDamageType::StaticClass());

    if (Victim->GetMeleeImpactSound())
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Victim->GetMeleeImpactSound(),
            GetActorLocation()
        );
    }
}

void AEnemy::SpawnBlood(AShooterCharacter* Character, FName SocketName)
{
    const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };
    if (TipSocket)
    {
        if (Character->GetBloodParticles())
        {
            const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), 
                Character->GetBloodParticles(), 
                SocketTransform);
        }
    }
}
