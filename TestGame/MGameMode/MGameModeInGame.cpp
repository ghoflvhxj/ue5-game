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
#include "MyPlayerState.h"
#include "CharacterLevelSubSystem.h"

void AMGameModeInGame::BeginPlay()
{
	Super::BeginPlay();

	if (UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>())
	{
		GameInstance->IterateItemTable([this](const FGameItemTableRow& GameItemTableRow) {
			if (GameItemTableRow.GameItemInfo.ItemType != EItemType::Common)
			{
				return;
			}

			ItemPool.Add(GameItemTableRow.Index);
		});
	}

	int32 NumItems = ItemPool.Num();
	for (int i = 0; i < NumItems; ++i)
	{
		Swap(ItemPool[i], ItemPool[FMath::Rand() % NumItems]);
	}
	
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

			AMPlayerState* PlayerState = PlayerCharacter->GetPlayerState<AMPlayerState>();
			if (IsValid(PlayerState) == false)
			{
				return;
			}

			switch (NewValue)
			{
				case (uint8)ECharacterVitalityState::Die:
				{
					PlayerState->Die();

					//GameStateInGame->AddDeadPlayer(PlayerState);
					APlayerController* PlayerController = PlayerState->GetPlayerController();
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
						PlayerController->StartSpectatingOnly();

						//FTimerHandle DummyHandle;
						//GetWorldTimerManager().SetTimer(DummyHandle, FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]() {
						//	PlayerController->ServerViewNextPlayer();
						//}), 5.f, false);
						
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

void AMGameModeInGame::PopItem(AActor* Popper, AActor* PopInstigator)
{
	if (ItemPool.IsEmpty())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("PopItem failed. Empty pool."));
		return;
	}

	FTransform Transform = FTransform::Identity;
	if (IsValid(Popper))
	{
		Transform = Popper->GetActorTransform();
	}
	else if(IsValid(PopInstigator))
	{
		Transform = PopInstigator->GetActorTransform();
	}
	else
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("Drop item failed. Invalid Dropper."));
		return;
	}

	int32 ItemIndex = ItemPool.Pop();
	SpawnItem(ItemIndex, Transform);
}

void AMGameModeInGame::DropItem(AActor* Dropper)
{
	if (IsValid(Dropper) == false)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("Drop item failed. Invalid Dropper."));
		return;
	}

	int32 ItemIndex = ItemIndices.Pop(false);

	SpawnItem(ItemIndex, Dropper->GetActorTransform());
}

void AMGameModeInGame::OnPawnKilled(APawn* Killer, APawn* Killed)
{
	bool bMonsterKilled = IsValid(Killed) && Killed->IsPlayerControlled() == false;

	//if (IsValid(Killer) && Killer->IsPlayerControlled() && bMonsterKilled)
	//{
	//	GetWorld()->GetSubsystem<UCharacterLevelSubSystem>()->AddExperiance(Killer, 10);
	//}

	//if (bMonsterKilled)
	//{
	//	FTransform Transform = Killed->GetActorTransform();
	//	Transform.SetScale3D(FVector::OneVector);
	//	
	//	if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this)))
	//	{
	//		if (FGameItemTableRow* MoneyItemInfo = GameInstance->GetItemTableRow(TEXT("Money")))
	//		{
	//			SpawnDropItem(MoneyItemInfo->Index, Transform);
	//		}
	//	}
	//}
}

void AMGameModeInGame::SpawnItem(int32 InItemIndex, const FTransform& InTransform)
{
	if (IsValid(DropItemClass) == false)
	{
		return;
	}

	FTransform Transform = InTransform;
	Transform.SetScale3D(FVector::OneVector);

	if (AItemBase* ItemActor = Cast<AItemBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, DropItemClass, Transform)))
	{
		ItemActor->SetItemIndex(InItemIndex);
		UGameplayStatics::FinishSpawningActor(ItemActor, Transform);
	}
}

bool AMGameModeInGame::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if (AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>())
	{
		return GameStateInGame->IsRevivalable() && Super::PlayerCanRestart_Implementation(Player);
	}

	return Super::PlayerCanRestart_Implementation(Player);
}

