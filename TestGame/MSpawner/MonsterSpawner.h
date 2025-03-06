// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "MonsterSpawner.generated.h"

class URoundComponent;
struct FRound;

UENUM(BlueprintType)
enum class ESpawnerType : uint8
{
	None,
	Monster,
};

USTRUCT(BlueprintType)
struct FSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	bool IsValid() const
	{
		return SpawnClass != nullptr;
	}

public:
	UPROPERTY(BlueprintReadOnly)
	UClass* SpawnClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SpawnTags;

	FTransform Transform;
};	

USTRUCT(BlueprintType)
struct FSpawnInfos : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSpawnInfo> SpawnInfos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, int32> MonsterIndexToSpawnNum;
};

USTRUCT(BlueprintType)
struct FRoundMonsterSpawnTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, int32> MonsterIndexToSpawnNum;
};

UCLASS()
class TESTGAME_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ASpawner();

public:
	virtual void Tick(float DeltaTime) override;

public:
	virtual void AddSpawnInfo(const FSpawnInfo& InSpawnInfo);
	virtual void AddSpawnInfo(const TArray<FSpawnInfo>& InSpawnInfos);
	virtual FTransform GetSpawnTransform();
	virtual void OnSpawnedActorConstruction(AActor* SpawnedActor) {}
	virtual void OnSpawned(AActor* SpawnedActor) {}

public:
	UFUNCTION(BlueprintPure)
	const TSet<AActor*>& GetSpawnedActors();
protected:
	TArray<TSubclassOf<AActor>> SpawneeClassArray;
	TSet<AActor*> SpawnedActors;
	TArray<FSpawnInfo> SpawnPool;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActivated = true;

	float LastSpawnTime = 0.f;

public:
	ESpawnerType GetSpawnerType() const { return SpawnerType; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ESpawnerType SpawnerType = ESpawnerType::None;
};

UCLASS()
class TESTGAME_API ARoundSpanwer : public ASpawner
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	virtual void SpawnUsingRoundInfo(const FRound& InRound);

protected:
	URoundComponent* GetRoundComponent();
	FRound CachedRound;
};

UCLASS()
class TESTGAME_API AMonsterSpawner : public ARoundSpanwer
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

public:
	virtual void SpawnUsingRoundInfo(const FRound& InRound) override;
	virtual void OnSpawnedActorConstruction(AActor* SpawnedActor) override;
	virtual void OnSpawned(AActor* SpawnedActor) override;
	virtual FTransform GetSpawnTransform() override;
public:
	UFUNCTION()
	void RemoveSpawnedActor(AActor* Actor, EEndPlayReason::Type EndPlayReason);

public:
	FRoundMonsterSpawnTableRow* GetRoundMonsterSpawn(const FRound& InRound);
	FRoundMonsterSpawnTableRow* GetCurrentRoundMonsterSpawn(int32 RoundOffest = 0);
	void LoadRoundMonsters(int32 InRound);
protected:
	TArray<int32> MonsterIndexPool;

public:
	DECLARE_EVENT_OneParam(AMonsterSpawner, FOnBossSpawnedEvent, AActor* /*BossMonster*/)
	FOnBossSpawnedEvent OnBossSpawnedEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* MonsterSpawnTable = nullptr;
	int32 LastSpawnWave = INDEX_NONE;
	int32 LastMonsterIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere)
	float MinRadius = 1000.f;
	UPROPERTY(EditAnywhere)
	float MaxRadius = 1500.f;

public:
	UFUNCTION(BlueprintPure)
	static bool IsBossContain(const UObject* WorldContext, const FRoundMonsterSpawnTableRow& InSpawnInfos);
protected:
	UPROPERTY(BlueprintReadOnly)
	AActor* BossMonster = nullptr;
	FGameplayTag BossTag = FGameplayTag::RequestGameplayTag("Monster.Grade.Boss");

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	int32 SpawnPositionNum = 0;
#endif

};
