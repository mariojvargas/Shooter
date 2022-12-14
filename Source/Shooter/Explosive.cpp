// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

// Sets default values
AExplosive::AExplosive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
    SetRootComponent(ExplosiveMesh);

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }

    if (ExplodeParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), 
            ExplodeParticles, 
            HitResult.Location, 
            FRotator(0.f), 
            true
        );
    }

    // Apply explosive damage
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (auto Actor : OverlappingActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor damaged by explosive [%s]"), *Actor->GetName());
    }

    Destroy();
}
