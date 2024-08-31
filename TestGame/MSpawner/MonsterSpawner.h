// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MonsterSpawner.generated.h"

UENUM(BlueprintType)
enum class ESpawnType : uint8
{
	None,
	Fixed,
	Probable,
};

USTRUCT(BlueprintType)
struct FSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	bool IsValid() const
	{
		return SpawnType != ESpawnType::None;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnType SpawnType = ESpawnType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnNum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AActor>, float> SpawneeClassMap;
};	

UCLASS()
class TESTGAME_API ASpawner : public AActor
{
	GENERATED_BODY()

public:
	virtual void Spawn(const FSpawnInfo& InSpawnInfo);
	virtual FTransform GetSpawnTransform();
	virtual void OnSpawned(AActor* SpawnedActor) {}

protected:
	TArray<TSubclassOf<AActor>> SpawneeClassArray;
	TArray<TWeakObjectPtr<AActor>> SpawnedActors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActivated = true;
};

UCLASS()
class TESTGAME_API ARoundSpanwer : public ASpawner
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	virtual void SpawnUsingRoundInfo(const FRoundInfo& InRoundInfo) {}
};

UCLASS()
class TESTGAME_API AMonsterSpawner : public ARoundSpanwer, public IRoundInterface
{
	GENERATED_BODY()

public:
	virtual void SpawnUsingRoundInfo(const FRoundInfo& InRoundInfo) override;
	virtual void OnSpawned(AActor* SpawnedActor) override;
	virtual FTransform GetSpawnTransform() override;
public:
	UFUNCTION()
	void RemoveSpawnedActor(AActor* Actor, EEndPlayReason::Type EndPlayReason);

public:
	bool IsClear_Implementation() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* MonsterSpawnTable;
};
