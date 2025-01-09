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

ASpectatorPawn* AMPlayerControllerInGame::SpawnSpectatorPawn()
{
	if (ShouldKeepCurrentPawnUponSpectating() == false)
	{
		return Super::SpawnSpectatorPawn();
	}

	return nullptr;
}

void AMPlayerControllerInGame::BeginSpectatingState()
{
	Super::BeginSpectatingState();

	if (IsLocalController() == false)
	{
		ClientGotoState(NAME_Spectating);
	}
}

void AMPlayerControllerInGame::EndSpectatingState()
{
	Super::EndSpectatingState();

	if (IsLocalController())
	{
		OnSpectateModeChangedEvent.Broadcast(false);
	}
}

void AMPlayerControllerInGame::StartSpectate()
{
	ServerViewNextPlayer();
}

void AMPlayerControllerInGame::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	APawn* PlayerPawn = GetPawn();
	if (IsValid(LocalPlayer) && IsValid(PlayerPawn))
	{
		if (IsInViewportClient(LocalPlayer->ViewportClient))
		{
			YawToMouseFromPawn = YawToMouseFromWorldLocation(PlayerPawn->GetActorLocation());

			if (FMath::IsNearlyEqual(LastSendYawToMouseFromPawn, YawToMouseFromPawn, 0.1f) == false)
			{
				Server_SetYawToMouse(YawToMouseFromPawn);
				LastSendYawToMouseFromPawn = YawToMouseFromPawn;
				UE_LOG(LogTemp, VeryVerbose, TEXT("Client send yaw to server."));
			}
		}
	}

	AActor* NewPicking = CurrentClickablePrimitive.IsValid() ? CurrentClickablePrimitive->GetOwner() : nullptr;
	if (IsValid(PickComponent))
	{
		PickComponent->SetPickingActor(NewPicking);
	}
}

void AMPlayerControllerInGame::SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams /*= FViewTargetTransitionParams()*/)
{
	AActor* OldViewTarget = GetViewTarget();

	Super::SetViewTarget(NewViewTarget, TransitionParams);

	if (OldViewTarget != NewViewTarget)
	{
		if (IsInState(NAME_Spectating))
		{
			OnSpectateModeChangedEvent.Broadcast(true);
		}
		OnViewTargetChangedEvent.Broadcast(OldViewTarget, NewViewTarget);
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

void AMPlayerControllerInGame::Server_SetYawToMouse_Implementation(float InYaw)
{
	YawToMouseFromPawn = InYaw;
}

float AMPlayerControllerInGame::YawToMouseFromWorldLocation(const FVector& InLocation)
{
	FVector MouseWorldLocation;
	FVector MouseWorldDirection;
	FCollisionQueryParams QueryParams;
	FCollisionResponseParams ResponsParams(ECollisionResponse::ECR_Block);

	if (DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
	{
		TArray<FHitResult> HitResults;
		if (GetWorld()->LineTraceMultiByChannel(HitResults, MouseWorldLocation, MouseWorldLocation + MouseWorldDirection * 10000.f, ECC_Visibility, QueryParams, ResponsParams))
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