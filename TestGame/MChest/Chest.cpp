// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chest.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "TestGame/MGameMode/MGameModeInGame.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea_Default.h"

ADestructableActor::ADestructableActor()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	GeometryCollectionComponent->SetupAttachment(BoxComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(BoxComponent);
}

void ADestructableActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);


}

void ADestructableActor::Destruct_Implementation(AActor* Desturctor)
{
	if (AMGameModeInGame* GameMode = Cast<AMGameModeInGame>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->PopItem(this, Desturctor);
	}

	if (IsValid(SphereComponent))
	{
		SphereComponent->SetGenerateOverlapEvents(false);
		SphereComponent->SetCanEverAffectNavigation(false);
		ClearComponentOverlaps();
	}
}
