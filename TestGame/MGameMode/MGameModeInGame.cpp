// Copyright Epic Games, Inc. All Rights Reserved.


#include "MGameModeInGame.h"
#include "OnlineSubsystem.h"
#include "Gameframework/PlayerState.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MCharacterEnum.h"

void AMGameModeInGame::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("GAAMEMODE BeginPlay"));
}

void AMGameModeInGame::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerPawn);
	ensure(IsValid(PlayerCharacter));

	PlayerCharacter->AddVitalityChangedDelegate (this, [this, PlayerCharacter](uint8 OldValue, uint8 NewValue) {
		AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>();
		ensure(IsValid(GameStateInGame));

		APlayerState* PlayerState = PlayerCharacter->GetPlayerState();
		ensure(IsValid(PlayerState));

		switch (NewValue)
		{
		case (uint8)ECharacterVitalityState::Die:
		{
			PlayerState->GetPlayerController()->UnPossess();

			GameStateInGame->AddDeadPlayer(PlayerState);
			if (PlayerCanRestart(PlayerState->GetPlayerController()))
			{
				RestartPlayer(PlayerState->GetPlayerController());
			}
			else
			{
				GameStateInGame->Multicast_GameOver();
			}
		}
		break;
		case (uint8)ECharacterVitalityState::Alive:
			
			break;
		}
	});
}

void AMGameModeInGame::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>();
	ensure(IsValid(GameStateInGame));

	APlayerController* PlayerController = Cast<APlayerController>(NewPlayer);
	ensure(IsValid(PlayerController));

	GameStateInGame->RevivePlayer(PlayerController->GetPlayerState<APlayerState>());
}

bool AMGameModeInGame::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if (Super::PlayerCanRestart_Implementation(Player) == false)
	{
		return false;
	}

	AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>();
	ensure(IsValid(GameStateInGame));

	if (GameStateInGame->IsRevivalable() == false)
	{
		return false;
	}

	return true;
}