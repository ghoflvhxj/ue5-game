// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MGameMode.h"

#include "MGameModeInGame.generated.h"

class APlayerState;

UCLASS()
class TESTGAME_API AMGameModeInGame : public AMGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
};
