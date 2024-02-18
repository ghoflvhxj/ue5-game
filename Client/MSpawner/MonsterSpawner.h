// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MonsterSpawner.generated.h"

USTRUCT(BlueprintType)
struct FSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> SpawnClass;
};	

UCLASS()
class CLIENT_API ASpawner : public AActor
{
	GENERATED_BODY()

public:
	virtual AActor* Spawn();
	virtual bool MakeSpawnInfo(FSpawnInfo& SpawnInfo);
	virtual bool MakeSpawnTransform(FTransform& Transfrom);
};

UCLASS()
class CLIENT_API AMonsterSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	//virtual AActor* Spawn() override;
	virtual bool MakeSpawnInfo(FSpawnInfo& SpawnInfo);
	virtual bool MakeSpawnTransform(FTransform& Transform) override;
	

	TArray<AActor*> SpawnedMonsters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* SpawnTable;
};
