#include "MPlayerController.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MHud/MHud.h"
#include "TestGame/MItem/DropItem.h" // 피킹 테스트를 위한 임시 Include

DECLARE_LOG_CATEGORY_CLASS(LogPick, Log, Log);

AMPlayerControllerInGame::AMPlayerControllerInGame()
{
	PickComponent = CreateDefaultSubobject<UPickComponent>(TEXT("PickComponent"));
}

void AMPlayerControllerInGame::BeginPlay()
{
	Super::BeginPlay();

	if (IsNetMode(NM_Standalone))
	{
		OnRep_PlayerState();
	}
	
	if (IsLocalController())
	{
		Server_Ready();
	}

	AMHudInGame* HudInGame = GetHUD<AMHudInGame>();
	if (IsValid(PickComponent) && IsValid(HudInGame))
	{
		PickComponent->GetPickChangedEvent().AddUObject(HudInGame, &AMHudInGame::TogglePickInfo);
		PickComponent->GetPickDataChangedEvent().AddUObject(HudInGame, &AMHudInGame::UdpatePickInfo);
	}
}

void AMPlayerControllerInGame::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	AActor* NewPicking = CurrentClickablePrimitive.IsValid() ? CurrentClickablePrimitive->GetOwner() : nullptr;
	if (IsValid(PickComponent))
	{
		PickComponent->SetPickingActor(NewPicking);
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

void AMPlayerControllerInGame::Server_Ready_Implementation()
{
	bReady = true;
}

UPickComponent::UPickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UPickComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PickingActor.IsValid() == false)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (APlayerController* PlayerController = GetOwner<APlayerController>())
	{
		FPickData PickData;
		PickData.WorldLocation = PickingActor->GetActorLocation();
		PickData.ScreenLocation = FVector2D::ZeroVector;
		if (PlayerController->ProjectWorldLocationToScreen(PickData.WorldLocation, PickData.ScreenLocation))
		{
			OnPickDataChangedEvent.Broadcast(PickData);
		}
	}
}

bool UPickComponent::IsPicking(AActor* InActor)
{
	return PickingActor.IsValid() && InActor == PickingActor;
}

bool UPickComponent::IsPickable(AActor* InActor)
{
	if (IsValid(InActor))
	{
		if (InActor->IsA<ADropItem>() == false)
		{
			return false;
		}
	}

	return true;
}

void UPickComponent::SetPickingActor(AActor* InActor)
{
	if (IsPickable(InActor) == false)
	{
		InActor = nullptr;
	}

	if (InActor == PickingActor)
	{
		return;
	}

	AActor* Old = PickingActor.Get();
	PickingActor = InActor;
	OnPickChangedEvent.Broadcast(Old, PickingActor.Get());

	SetComponentTickEnabled(PickingActor.IsValid());

	UE_LOG(LogTemp, Warning, TEXT("NEwPicking Valid:%d"), (int)IsValid(InActor));
}