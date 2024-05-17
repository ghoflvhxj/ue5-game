#include "MPlayerController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "TestGame/MCharacter/MCharacter.h"

AMPlayerControllerInGame::AMPlayerControllerInGame()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMPlayerControllerInGame::BeginPlay()
{
	Super::BeginPlay();

	AddInputMappingContext();
}

FVector AMPlayerControllerInGame::GetMouseWorldPosition()
{
	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this);

	if (IsValid(NavigationSystem) == false)
	{
		return FVector::ZeroVector;
	}

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	return HitResult.bBlockingHit ? HitResult.Location : FVector::ZeroVector;
}


void AMPlayerControllerInGame::Server_PawnMoveToLocation_Implementation(const FVector& Location)
{
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Location);
}

void AMPlayerControllerInGame::AddInputMappingContext_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

//void AMPlayerControllerTitle::BeginPlay()
//{
//	//PlayerCameraManager->SetManualCameraFade(1.f, )
//}
