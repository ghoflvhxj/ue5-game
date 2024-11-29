// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGameModeInGame.h"
#include "OnlineSubsystem.h"
#include "Gameframework/PlayerState.h"
#include "MGameInstance.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MChest/Chest.h"
#include "TestGame/MItem/DropItem.h"
#include "TestGame/MCharacter/MCharacterEnum.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "CharacterLevelSubSystem.h"

void AMGameModeInGame::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("GAAMEMODE BeginPlay"));
}

void AMGameModeInGame::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerPawn);
	if (ensure(PlayerCharacter))
	{
		PlayerCharacter->AddVitalityChangedDelegate(this, [this, PlayerCharacter](uint8 OldValue, uint8 NewValue) {
			AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>();
			if (IsValid(GameStateInGame) == false)
			{
				return;
			}

			APlayerState* PlayerState = PlayerCharacter->GetPlayerState();
			if (IsValid(PlayerState) == false)
			{
				return;
			}

			switch (NewValue)
			{
				case (uint8)ECharacterVitalityState::Die:
				{
					APlayerController* PlayerController = PlayerState->GetPlayerController();
					GameStateInGame->AddDeadPlayer(PlayerState);
					if (PlayerCanRestart(PlayerController))
					{
						FTimerHandle DummyHandle;
						PlayerCharacter->GetWorldTimerManager().SetTimer(DummyHandle, FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]() {
							PlayerController->UnPossess();
							RestartPlayer(PlayerController);
							}), 5.f, false);
					}
					else
					{
						//GameStateInGame->Multicast_GameOver();
					}
				}
				break;
				case (uint8)ECharacterVitalityState::Alive:
				{

				}
				break;
			}
		});
	}
}

void AMGameModeInGame::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (IsValid(NewPlayer))
	{
		if (AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>())
		{
			GameStateInGame->RevivePlayer(NewPlayer->GetPlayerState<APlayerState>());
		}
	}
}

bool AMGameModeInGame::ReadyToEndMatch_Implementation()
{
	if (AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>())
	{
		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			return RoundComponent->IsFinished();
		}
	}

	return false;
}

void AMGameModeInGame::OnActorDestruct(ADestructableActor* InDestructableActor)
{
	if (IsValid(InDestructableActor))
	{
		FTransform Transform = InDestructableActor->GetActorTransform();
		Transform.SetScale3D(FVector::OneVector);

		auto PickItemIndex = []()->int32 {
			return FMath::RandRange(0, 3);
		};

		int32 ItemIndex = PickItemIndex();
		SpawnDropItem(ItemIndex, Transform);

		// GameItemComponent->SpawnRandomItem();
	}
}

void AMGameModeInGame::OnPawnKilled(APawn* Killer, APawn* Killed)
{
	bool bMonsterKilled = IsValid(Killed) && Killed->IsPlayerControlled() == false;

	//if (IsValid(Killer) && Killer->IsPlayerControlled() && bMonsterKilled)
	//{
	//	GetWorld()->GetSubsystem<UCharacterLevelSubSystem>()->AddExperiance(Killer, 10);
	//}

	if (bMonsterKilled)
	{
		FTransform Transform = Killed->GetActorTransform();
		Transform.SetScale3D(FVector::OneVector);
		
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (FItemBaseInfo* MoneyItemInfo = GameInstance->GetItemBaseInfo(TEXT("Money")))
			{
				SpawnDropItem(MoneyItemInfo->Index, Transform);
			}
		}
	}
}

ADropItem* AMGameModeInGame::SpawnDropItem(int32 InItemIndex, FTransform& InTransform)
{
	if (IsValid(DropItemClass))
	{
		if (ADropItem* DropItem = Cast<ADropItem>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, DropItemClass, InTransform)))
		{
			DropItem->SetItemIndex(InItemIndex);
			UGameplayStatics::FinishSpawningActor(DropItem, InTransform);
		}
	}

	return nullptr;
}

bool AMGameModeInGame::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if (AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>())
	{
		return GameStateInGame->IsRevivalable() && Super::PlayerCanRestart_Implementation(Player);
	}

	return Super::PlayerCanRestart_Implementation(Player);
}