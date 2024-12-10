#include "MPlayerController.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MHud/MHud.h"

void AMPlayerControllerInGame::BeginPlay()
{
	Super::BeginPlay();

	if (IsNetMode(NM_Standalone))
	{
		OnRep_PlayerState();
	}
}

void AMPlayerControllerInGame::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AMHud* Hud = GetHUD<AMHud>())
	{
		Hud->InitializeUsingPlayerState(GetPlayerState<APlayerState>());
	}
}

float AMPlayerControllerInGame::GetAngleToMouse(const FVector& InLocation)
{
	FVector MouseWorldLocation;
	FVector MouseWorldDirection;
	FCollisionObjectQueryParams CollsionParam;
	CollsionParam.AddObjectTypesToQuery(ECC_WorldStatic);
	CollsionParam.AddObjectTypesToQuery(ECC_WorldDynamic);

	if (DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
	{
		TArray<FHitResult> HitResults;
		if (GetWorld()->LineTraceMultiByObjectType(HitResults, MouseWorldLocation, MouseWorldLocation + MouseWorldDirection * 10000.f, CollsionParam))
		{
			return UKismetMathLibrary::FindLookAtRotation(InLocation, HitResults[0].Location).Yaw;
		}
	}

	return 0.f;
}

//FVector AMPlayerControllerInGame::GetMouseWorldPosition()
//{
//	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this);
//
//	if (IsValid(NavigationSystem) == false)
//	{
//		return FVector::ZeroVector;
//	}
//
//	FHitResult HitResult;
//	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
//
//	return HitResult.bBlockingHit ? HitResult.Location : FVector::ZeroVector;
//}

//void AMPlayerControllerTitle::BeginPlay()
//{
//	//PlayerCameraManager->SetManualCameraFade(1.f, )
//}
