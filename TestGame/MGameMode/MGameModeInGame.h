// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "MGameModeInGame.generated.h"


UCLASS()
class TESTGAME_API AMGameModeInGame : public AGameMode
{
	GENERATED_BODY()

	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
};
