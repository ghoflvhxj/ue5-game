// Copyright Epic Games, Inc. All Rights Reserved.


#include "MGameModeInGame.h"
#include "OnlineSubsystem.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"

void AMGameModeInGame::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("GAAMEMODE BeginPlay"));
}

void AMGameModeInGame::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerPawn);
	ensure(IsValid(PlayerCharacter));

	AGameMode* GameMode = this;

	PlayerCharacter->AddVitalityChangedDelegate (this, [this](uint8 OldValue, uint8 NewValue) {
		if (NewValue == static_cast<uint8>(ECharacterVitalityState::Die))
		{
			AMGameStateInGame* GameStateInGame = GetGameState<AMGameStateInGame>();
			ensure(IsValid(GameStateInGame));

			GameStateInGame->Multicast_GameOver();
		}
	});
}
