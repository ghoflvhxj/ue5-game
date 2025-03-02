// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MGameMode.h"

#include "MGameModeInGame.generated.h"

class APlayerState;
class ADestructableActor;
class ADropItem;

UCLASS()
class TESTGAME_API AMGameModeInGame : public AMGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
public:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void HandleMatchHasStarted() override;
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

public:
	void PopItem(const FVector& InLocation, AActor* PopInstigator);
	void DropItem(AActor* Dropper);
	UFUNCTION(BlueprintCallable)
	void DropItem(AActor* InDroppoer, int32 InIndex);
	void OnPawnKilled(APawn* Killer, APawn* Killed);
protected:
	void SpawnItem(int32 InItemIndex, const FTransform& InTransform);
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ADropItem> DefaultDropItemClass = nullptr;
	TArray<int32> ItemPool;
};
