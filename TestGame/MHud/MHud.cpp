#include "MHud.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MSpawner/MonsterSpawner.h"
#include "TestGame/MPlayerController/MPlayerController.h"
#include "MyPlayerState.h"
#include "AttributeDisplayWidget.h" // ActorBindWidget 참조
//#include "CharacterLevelSubSystem.h"

void AMHud::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget(GetWorld(), HUDWidgetClass);
	}

	if (IsValid(HUDWidget) && ShowHudWidgetAfterCreation)
	{
		ShowWidget();
	}
}

bool AMHud::InitializeUsingPlayerState(APlayerState* PlayerState)
{
	return IsValid(PlayerState);
}

void AMHud::ShowWidget()
{
	FTimerHandle DummyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DummyTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (IsValid(HUDWidget))
		{
			HUDWidget->AddToViewport();
		}
	}), ShowTimer + 1.f, false, -1.f);
}

void AMHudInGame::ShowGameOver_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!!!"));
}

AMHudInGame::AMHudInGame()
{
	ShowHudWidgetAfterCreation = false;
}

void AMHudInGame::BeginPlay()
{
	Super::BeginPlay();

	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			UpdateRoundInfo(RoundComponent->GetRoundWave());
			RoundComponent->GetWaitNextRoundEvent().AddUObject(this, &AMHudInGame::ShowGetRewardMessage);
			RoundComponent->OnRoundAndWaveChangedEvent.AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}

		GameStateInGame->OnBossMonsterSet.AddUObject(this, &AMHudInGame::BoundBoss);
		GameStateInGame->OnMatchEndEvent.AddUObject(this, &AMHudInGame::ShowGameFinish);
	}

	if (AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(PlayerOwner))
	{
		UpdatePawnBoundWidget(nullptr, PlayerOwner->GetPawn());
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::UpdatePawnBoundWidget);
		PlayerController->GetSpectateModeChangedEvent().AddWeakLambda(this, [this, PlayerController](bool bSpectateMode) {
			UE_LOG(LogTemp, Warning, TEXT("Change To Spectate"));
			if (bSpectateMode)	
			{
				UpdatePawnBoundWidget(nullptr, Cast<APawn>(PlayerController->GetViewTarget()));
			}
		});
		PlayerController->GetViewTargetChangedEvent().AddWeakLambda(this, [this, PlayerController](AActor* Old, AActor* New) {
			if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
			{
				UE_LOG(LogTemp, Warning, TEXT("PlayerState Spectate:%d"), (int)PlayerState->IsSpectator());
				if (PlayerState->IsSpectator())
				{
					UpdatePawnBoundWidget(nullptr, Cast<APawn>(PlayerController->GetViewTarget()));
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Change Viewtarget To %s"), PlayerController->GetViewTarget() ? *PlayerController->GetViewTarget()->GetName() : *FString("None"));
		});

		PlayerController->GetSpectateModeChangedEvent().AddUObject(this, &AMHudInGame::ShowSpectateInfo);
	}
}

bool AMHudInGame::InitializeUsingPlayerState(APlayerState* InPlayerState)
{
	if (Super::InitializeUsingPlayerState(InPlayerState))
	{
		if (UMInventoryComponent* InventoryComponent = InPlayerState->GetComponentByClass<UMInventoryComponent>())
		{
			InventoryComponent->OnMoneyChangedEvent.AddUObject(this, &AMHudInGame::UpdateMoney);
		}

		if (AMPlayerState* PlayerState = Cast<AMPlayerState>(InPlayerState))
		{
			PlayerState->GetPlayerDeadEvent().AddUObject(this, &AMHudInGame::ShowDieInfo);
		}

		return true;
	}

	return false;
}

void AMHudInGame::UpdatePawnBoundWidget(APawn* OldPawn, APawn* NewPawn)
{
	Super::UpdatePawnBoundWidget(OldPawn, NewPawn);

	if (IsValid(HUDWidget))
	{
		if (UWidgetTree* WidgetTree = HUDWidget->WidgetTree)
		{
			WidgetTree->ForWidgetAndChildren(HUDWidget->GetRootWidget(), [this, NewPawn](UWidget* Widget) {
				if (UActorBindWidget* ActorBindWidget = Cast<UActorBindWidget>(Widget))
				{
					ActorBindWidget->BindActor(NewPawn);
				}
			});
		}
	}
}