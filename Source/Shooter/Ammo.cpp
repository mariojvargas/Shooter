// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AAmmo::AAmmo()
{
    AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
    SetRootComponent(AmmoMesh);

    GetCollisionBox()->SetupAttachment(GetRootComponent());
    GetPickupWidget()->SetupAttachment(GetRootComponent());
    GetAreaSphere()->SetupAttachment(GetRootComponent());
}

void AAmmo::BeginPlay()
{
    Super::BeginPlay();    
}

void AAmmo::SetItemProperties(EItemState State)
{
    Super::SetItemProperties(State);

    switch (State)
	{
		case EItemState::EIS_Pickup:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AmmoMesh->SetEnableGravity(false);
			break;

		case EItemState::EIS_Equipped:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AmmoMesh->SetEnableGravity(false);
			break;

		case EItemState::EIS_Falling:
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			AmmoMesh->SetEnableGravity(true);
			break;

		case EItemState::EIS_EquipInterpolating:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AmmoMesh->SetEnableGravity(false);
			break;
	}
}

void AAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
