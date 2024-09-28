// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chest.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "TestGame/MGameMode/MGameModeInGame.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea_Default.h"

ADestructableActor::ADestructableActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	GeometryCollectionComponent->SetupAttachment(SphereComponent);
}

void ADestructableActor::Destruct_Implementation()
{
	if (AMGameModeInGame* GameMode = Cast<AMGameModeInGame>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnActorDestruct(this);
	}

	if (IsValid(SphereComponent))
	{
		SphereComponent->SetGenerateOverlapEvents(false);
		SphereComponent->SetCanEverAffectNavigation(false);
		ClearComponentOverlaps();
	}
}
