// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineStats.h"

#include "ClientGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CLIENT_API AClientGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void LeaderboardTest();
	FOnlineLeaderboardReadPtr ReadObject;
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;
	void PrintLeaderboard(bool b);
};
