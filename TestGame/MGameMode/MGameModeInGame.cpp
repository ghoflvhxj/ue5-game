// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGameModeInGame.h"
#include "OnlineSubsystem.h"
#include "Gameframework/PlayerState.h"
#include "MGameInstance.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MChest/Chest.h"
#include "TestGame/MItem/Drop.h"
#include "TestGame/MItem/DropItem.h"
#include "TestGame/MCharacter/MCharacterEnum.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MPlayerController/MPlayerController.h"
#include "MyPlayerState.h"
#include "CharacterLevelSubSystem.h"

void AMGameModeInGame::BeginPlay()
{
	Super::BeginPlay();

	if (UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>())
	{
		GameInstance->IterateItemTable([this](const FGameItemTableRow& GameItemTableRow) {
			if (GameItemTableRow.GameItemInfo.ItemType != EItemType::Item)
			{
				return;
			}

			int32 ItemMaxLevel = GameItemTableRow.GameItemData.GetMaxLevel();
			for (int32 i = 0; i < ItemMaxLevel; ++i)
			{
				ItemPool.Add(GameItemTableRow.Index);
			}
		});
	}

	int32 NumItems = ItemPool.Num();
	for (int i = 0; i < NumItems; ++i)
	{
		Swap(ItemPool[i], ItemPool[FMath::Rand() % NumItems]);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GAAMEMODE BeginPlay"));
}

void AMGameModeInGame::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (HasMatchStarted())
	{
		ErrorMessage += TEXT("Game already started, Can't join.");
	}
}

void AMGameModeInGame::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerPawn);
	if (ensure(PlayerCharacter))
	{
		PlayerCharacter->GetOnDeadEvent().AddWeakLambda(this, [this, PlayerCharacter](AActor* InActor) {
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

			PlayerState->Die();

			if (APlayerController* PlayerController = PlayerState->GetPlayerController())
			{
				if (PlayerCanRestart(PlayerController))
				{
					//RestartPlayer(PlayerController); -> Respawn 함수를 만들어서 호출
				}
				else
				{
					if (IsValid(PlayerController))
					{
						PlayerController->StartSpectatingOnly();
					}
				}
			}

		});
	}
}

void AMGameModeInGame::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void AMGameModeInGame::HandleMatchHasStarted()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		RestartPlayer(It->Get());
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PlayerController = It->Get())
		{
			if (AMCharacter* PlayerCharacter = PlayerController->GetPawn<AMCharacter>())
			{
				PlayerCharacter->PlayStartAnim();
			}
		}
	}
}

bool AMGameModeInGame::ReadyToStartMatch_Implementation()
{
	if (Super::ReadyToStartMatch_Implementation())
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(It->Get()))
			{
				if (PlayerController->IsReady() == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	return false;
}

bool AMGameModeInGame::ReadyToEndMatch_Implementation()
{
	if (AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>())
	{
		bool bAllRoundFinished = false;
		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			bAllRoundFinished = RoundComponent->IsFinished();
		}

		bool bAllPlayerDead = GameStateInGame->IsAllPlayerDead();

		return bAllRoundFinished || bAllPlayerDead;
	}

	return false;
}

UClass* AMGameModeInGame::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(InController);

	if (IsValid(PlayerController))
	{
		const FPlayerCharacterTableRow& PlayerCharTableRow = UMGameInstance::GetPlayerCharacterTableRow(GetWorld(), PlayerController->GetCharacterIndex());
		if (IsValid(PlayerCharTableRow.PlayerCharacterClass) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Using Invalid PlayerCharacterIndex[%d]."), PlayerController->GetCharacterIndex());
		}

		return PlayerCharTableRow.PlayerCharacterClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AMGameModeInGame::PopItem(const FVector& InLocation, AActor* PopInstigator)
{
	if (ItemPool.IsEmpty())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("PopItem failed. Empty pool."));
		return;
	}

	FTransform Transform = FTransform::Identity;
	Transform.SetLocation(InLocation);

	//if(IsValid(PopInstigator))
	//{
	//	Transform = PopInstigator->GetActorTransform();
	//}
	//else
	//{
	//	UE_LOG(LogTemp, VeryVerbose, TEXT("Pop failed."));
	//	return;
	//}

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

	TMap<int32, float> TestProbMap;
	TestProbMap.Add(5, 0.7f);

	int32 ItemIndex = INDEX_NONE;
	float RandomFloat = FMath::FRand();

	for (const TPair<int32, float>& Pair : TestProbMap)
	{
		if (RandomFloat < Pair.Value)
		{
			ItemIndex = Pair.Key;
			break;
		}

		RandomFloat -= Pair.Value;
	}

	SpawnItem(ItemIndex, Dropper->GetActorTransform());
}

void AMGameModeInGame::DropItem(const FTransform& InTransform, int32 InIndex)
{
	const FDropTableRow& DropTableRow = UMGameInstance::GetDropTableRow(this, InIndex);

	int32 ItemIndex = INDEX_NONE;
	float RandomFloat = FMath::FRand();
	for (const TPair<int32, float>& Pair : DropTableRow.Porbs)
	{
		if (RandomFloat < Pair.Value)
		{
			ItemIndex = Pair.Key;
			break;
		}

		RandomFloat -= Pair.Value;
	}

	SpawnItem(ItemIndex, InTransform);
}

void AMGameModeInGame::OnPawnKilled(APawn* Killer, APawn* Killed)
{
	bool bMonsterKilled = IsValid(Killed) && Killed->IsPlayerControlled() == false;

	if (IsValid(Killer) && Killer->IsPlayerControlled() && bMonsterKilled)
	{
		//GetWorld()->GetSubsystem<UCharacterLevelSubSystem>()->AddExperiance(Killer, 1);
	}

	if (bMonsterKilled)
	{
		if (Killed->GetClass()->ImplementsInterface(UDropInterface::StaticClass()))
		{
			DropItem(Killed->GetActorTransform(), IDropInterface::Execute_GetDropIndex(Killed));
		}
		else
		{
			DropItem(Killed);
		}
	}
}

void AMGameModeInGame::SpawnItem(int32 InItemIndex, const FTransform& InTransform)
{
	if (InItemIndex == INDEX_NONE)
	{
		return;
	}

	UClass* DropItemClass = DefaultDropItemClass;
	if (UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>())
	{
		if (FGameItemTableRow* ItemTableRow = GameInstance->GetItemTableRow(InItemIndex))
		{
			if (UClass* LoadedClass = ItemTableRow->GameItemInfo.DropItemClass.TryLoadClass<ADropItem>())
			{
				DropItemClass = LoadedClass;
			}
		}
	}

	if (IsValid(DropItemClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid DropItemClass."));
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

