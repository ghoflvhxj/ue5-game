// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chest.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea_Default.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsObjectBlueprintLibrary.h"

#include "TestGame/MComponents/DamageComponent.h"
#include "TestGame/MGameMode/MGameModeInGame.h"

ADestructableActor::ADestructableActor()
{
	//BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	//SetRootComponent(BoxComponent);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	GeometryCollectionComponent->SetupAttachment(SceneComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(SceneComponent);

	Tags.Add("Damagable");
}

void ADestructableActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority())
	{
		bool bDestruct = false;
		if (UMDamageComponent* DamageComponent = OtherActor->FindComponentByClass<UMDamageComponent>())
		{
			if (DamageComponent->IsReactable(this))
			{
				bDestruct = true;
			}
		}
		//else if (ADamageGiveActor* DamageGiveActor = Cast<ADamageGiveActor>(OtherActor))
		//{
		//	bDestruct = true;
		//}

		if (bDestruct)
		{
			GetWorldTimerManager().SetTimerForNextTick([this, OtherActor]() {
				Multicast_Destruct(OtherActor);
			});
		}
	}
}

void ADestructableActor::Multicast_Destruct_Implementation(AActor* Destructor)
{
	Destruct(Destructor);
}

void ADestructableActor::Destruct_Implementation(AActor* Desturctor)
{
	if (bDestructed)
	{
		return;
	}

	if (AMGameModeInGame* GameMode = Cast<AMGameModeInGame>(UGameplayStatics::GetGameMode(this)))
	{
		// 왜 링크 에러가 나는지 도저히 모르겠음
		//const FTransform& PhysicsObjectTransform = UPhysicsObjectBlueprintLibrary::GetPhysicsObjectWorldTransform(GeometryCollectionComponent, NAME_None);
		//GameMode->PopItem(PhysicsObjectTransform.GetLocation(), Desturctor);

		if (bUseDrop)
		{
			GameMode->DropItem(FTransform(GetItemSpawnLocation()), DropIndex);
		}
		else
		{
			GameMode->PopItem(GetItemSpawnLocation(), Desturctor);
		}
		
		SetLifeSpan(5.f);
	}

	if (IsValid(SphereComponent))
	{
		SphereComponent->SetGenerateOverlapEvents(false);
		SphereComponent->SetCanEverAffectNavigation(false);
		ClearComponentOverlaps();
	}

	Tags.Remove("Damagable");

	bDestructed = true;
}
