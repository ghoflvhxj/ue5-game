// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MonsterSpawner.generated.h"

struct FRound;

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
		return SpawnNum != 0;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnNum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AActor>, float> SpawneeClassMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GradeTag;
};	

USTRUCT(BlueprintType)
struct FSpawnInfos : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSpawnInfo> SpawnInfos;
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
	virtual void SpawnUsingRoundInfo(const FRound& InRound) {}
};

UCLASS()
class TESTGAME_API AMonsterSpawner : public ARoundSpanwer
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

public:
	virtual void SpawnUsingRoundInfo(const FRound& InRound) override;
	virtual void OnSpawned(AActor* SpawnedActor) override;
	virtual FTransform GetSpawnTransform() override;
public:
	UFUNCTION()
	void RemoveSpawnedActor(AActor* Actor, EEndPlayReason::Type EndPlayReason);

protected:
	URoundComponent* GetRoundComponent();

public:
	//bool IsClear_Implementation() override;

public:
	DECLARE_EVENT_OneParam(AMonsterSpawner, FOnBossSpawnedEvent, AActor* /*BossMonster*/)
	FOnBossSpawnedEvent OnBossSpawnedEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* MonsterSpawnTable = nullptr;
	int32 LastSpawnWave = INDEX_NONE;

	UPROPERTY(EditAnywhere)
	float MinRadius = 1000.f;
	UPROPERTY(EditAnywhere)
	float MaxRadius = 1500.f;

protected:
	UPROPERTY(BlueprintReadOnly)
	AActor* BossMonster = nullptr;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	int32 SpawnPositionNum = 0;
#endif

	UFUNCTION(BlueprintPure)
	static bool IsBossContain(const FSpawnInfos& InSpawnInfos)
	{
		FGameplayTag BossTag = FGameplayTag::RequestGameplayTag("Monster.Grade.Boss");
		for (const FSpawnInfo& SpawnInfo : InSpawnInfos.SpawnInfos)
		{
			if (SpawnInfo.GradeTag == BossTag)
			{
				return true;
			}
		}

		return false;
	}
};
