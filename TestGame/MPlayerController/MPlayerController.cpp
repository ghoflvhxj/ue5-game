#include "MPlayerController.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"

#include "MyPlayerState.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MHud/MHud.h"
#include "TestGame/MItem/DropItem.h" // 피킹 테스트를 위한 임시 Include

DECLARE_LOG_CATEGORY_CLASS(LogPick, Log, Log);

void AMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhacnedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhacnedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMPlayerController::CloseUIOrOpenSystem);
	}
}

void AMPlayerController::CloseUIOrOpenSystem()
{
	AMHud* MHud = Cast<AMHud>(GetHUD());
	if (IsValid(MHud) == false)
	{
		return;
	}

	MHud->CloseWidget();
}

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

	if (HasAuthority())
	{
		ClientGotoState(NAME_Spectating);
	}
	
	OnSpectateModeChangedEvent.Broadcast(true);
}

void AMPlayerControllerInGame::EndSpectatingState()
{
	Super::EndSpectatingState();

	if (HasAuthority())
	{
		OnSpectateModeChangedEvent.Broadcast(false);
	}

	OnSpectateModeChangedEvent.Broadcast(false);
}

void AMPlayerControllerInGame::Die()
{
	AMCharacter* PlayerCharacter = GetPawn<AMCharacter>();
	if (IsValid(PlayerCharacter) == false)
	{
		return;
	}

	AMPlayerState* MyPlayerState = GetPlayerState<AMPlayerState>();
	if (IsValid(MyPlayerState) == false)
	{
		return;
	}

	StartSpectatingOnly();
	MyPlayerState->Die();
}

void AMPlayerControllerInGame::StartSpectate()
{
	ServerViewNextPlayer();
}

void AMPlayerControllerInGame::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	APawn* PlayerPawn = GetPawn();

	if (IsValid(PlayerPawn) == false)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (IsInViewportClient(LocalPlayer->ViewportClient))
		{
			YawToMouse = YawToMouseFromWorldLocation(PlayerPawn->GetActorLocation());
			DirectionToMouseFromPawn = UKismetMathLibrary::RotateAngleAxis(FVector::ForwardVector, YawToMouse, FVector::UpVector);
			SetControlRotation(FRotator(0.f, YawToMouse, 0.f));

			if (FMath::IsNearlyEqual(LastSendYaw, YawToMouse, 0.1f) == false)
			{
				Server_SetYawToMouse(YawToMouse);
				LastSendYaw = YawToMouse;
			}
		}
	}

	// 피킹
	AActor* NewPicking = CurrentClickablePrimitive.IsValid() ? CurrentClickablePrimitive->GetOwner() : nullptr;
	if (IsValid(PickComponent))
	{
		PickComponent->SetPickingActor(NewPicking);
	}


	FVector CameraLocation;
	FRotator CameraRotation;
	GetPlayerViewPoint(CameraLocation, CameraRotation);

	TSet<TWeakObjectPtr<UPrimitiveComponent>> NewMaksedPrimitives;
	TArray<FHitResult> HitResults;
	if (GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, PlayerPawn->GetActorLocation(), ECC_Visibility, FCollisionQueryParams(), FCollisionResponseParams()) == false)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			if (UPrimitiveComponent* Primitive = HitResult.GetComponent())
			{
				NewMaksedPrimitives.Add(Primitive);
			}

			MaskedPrimitives.Append(NewMaksedPrimitives);
		}
	}

	for (auto Iter = MaskedPrimitives.CreateIterator(); Iter; ++Iter)
	{
		TWeakObjectPtr<UPrimitiveComponent>& Primitive = *Iter;
		if (Primitive.IsValid() == false)
		{
			Iter.RemoveCurrent();
			continue;
		}

		int32 NumMaterials = Primitive->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
		{
			UMaterialInterface* Material = Primitive->GetMaterial(MaterialIndex);
			if (UMaterialInstanceDynamic* DynamicMaterial = Primitive->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
			{
				FName ParamName = TEXT("Test");
				float Opacity = 0.f;
				DynamicMaterial->GetScalarParameterValue(ParamName, Opacity);
				Opacity += DeltaTime * 10.f * (NewMaksedPrimitives.Contains(Primitive.Get()) ? -1.f : 1.f);
				Opacity = FMath::Clamp(Opacity, 0.1f, 1.f);
				DynamicMaterial->SetScalarParameterValue(ParamName, Opacity);
			}
		}
	}
}

void AMPlayerControllerInGame::SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams /*= FViewTargetTransitionParams()*/)
{
	AActor* OldViewTarget = GetViewTarget();

	Super::SetViewTarget(NewViewTarget, TransitionParams);

	if (OldViewTarget != NewViewTarget)
	{
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

void AMPlayerControllerInGame::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (AMCharacter* PlayerCharacter = Cast<AMCharacter>(aPawn))
	{
		PlayerCharacter->GetOnDeadEvent().AddUObject(this, &AMPlayerControllerInGame::Die);
	}
}

void AMPlayerControllerInGame::Server_SetYawToMouse_Implementation(float InYaw)
{
	YawToMouse = InYaw;
	SetControlRotation(FRotator(0.f, YawToMouse, 0.f));
}

float AMPlayerControllerInGame::YawToMouseFromWorldLocation(const FVector& InLocation)
{
	FVector2D ScreenLocation;
	if (ProjectWorldLocationToScreen(InLocation, ScreenLocation))
	{
		FVector2D MouseLocation;
		GetMousePosition(MouseLocation.X, MouseLocation.Y);

		FVector2D ToMouse = MouseLocation - ScreenLocation;
		float Angle = FMath::RadiansToDegrees(FMath::Atan2(ToMouse.X, -ToMouse.Y)); // ATAN을 사용하기 위해 Y값을 반전하여 화면 좌표계를 일반적인 좌표계로 
		return Angle;
	}

	return 0.f;
}

FVector AMPlayerControllerInGame::GetDirectionToMouse()
{
	return IsLocalController() ? DirectionToMouseFromPawn : UKismetMathLibrary::RotateAngleAxis(FVector::ForwardVector, YawToMouse, FVector::UpVector);
}

void AMPlayerControllerInGame::SetCharacterIndex(int32 InIndex)
{
	// CharacterIndex Replication을 하지 않고, 서버와 클라 각각 관리. 이러면 HUD 등을 바로 갱신할 수 있음.
	if (AMPlayerState* MPlayerState = GetPlayerState<AMPlayerState>())
	{
		MPlayerState->SetCharacterIndex(InIndex);
	}

	Server_CharacterSelect(InIndex);
}

void AMPlayerControllerInGame::Server_CharacterSelect_Implementation(int32 InIndex)
{
	if (AMPlayerState* MPlayerState = GetPlayerState<AMPlayerState>())
	{
		MPlayerState->SetCharacterIndex(InIndex);
	}

	bReady = InIndex != INDEX_NONE;
}

int32 AMPlayerControllerInGame::GetCharacterIndex() const
{
	if (AMPlayerState* MPlayerState = GetPlayerState<AMPlayerState>())
	{
		return MPlayerState->GetCharacterIndex();
	}

	return INDEX_NONE;
}

void AMPlayerControllerInGame::Server_Ready_Implementation()
{
	bReady = true;
}

void AMPlayerControllerInGame::Client_SkillEnhance_Implementation(const FSkillEnhanceData& InEnhanceData)
{
	if (AMHudInGame* Hud = GetHUD<AMHudInGame>())
	{
		Hud->ShowSkillEnhanceWidget(InEnhanceData);
	}
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

		if (bPickInterface)
		{
			PickData.WorldLocation = IPickInterface::Execute_GetLocation(PickingActor.Get());
		}

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

	if (PickingActor.IsValid())
	{
		bPickInterface = PickingActor->GetClass()->ImplementsInterface(UPickInterface::StaticClass());
	}

	SetComponentTickEnabled(PickingActor.IsValid());

	UE_LOG(LogTemp, Warning, TEXT("NEwPicking Valid:%d"), (int)IsValid(InActor));
}
