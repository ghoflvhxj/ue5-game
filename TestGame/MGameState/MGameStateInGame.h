// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "MGameStateInGame.generated.h"

class APlayerState;
class AMonsterSpawner;

USTRUCT(BlueprintType)
struct FRoundInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Round = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveIntervalBase = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, float> WaveInterval;

public:
	bool IsValid() const { return Round != INDEX_NONE; }
};

USTRUCT(BlueprintType)
struct FRound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Wave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NextWaveTime = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStartedDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDynamicDelegate);
DECLARE_EVENT(AMGameStateInGame, FOnMatchEndEvent);

UCLASS()
class TESTGAME_API AMGameStateInGame : public AGameState
{
	GENERATED_BODY()

public:
	AMGameStateInGame();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URoundComponent* RoundComponent = nullptr;

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
public:
	FOnMatchEndEvent OnMatchEndEvent;
	FTimerHandle GameStartTimerHandle;

	// 부활
public:
	bool IsRevivalable();
	bool RevivePlayer(APlayerState* PlayerState);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PlayerLife;

	// 게임 오버
public:
	void AddDeadPlayer(APlayerState* DeadPlayerState);
	void RemoveDeadPlayer(APlayerState* DeadPlayerState);
	bool IsAllPlayerDead();
	
	TArray<APlayerState*> DeadPlayerArray;
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GameOver();
public:
	FOnGameOverDynamicDelegate GameOverDynamicDelegate;

public:
	DECLARE_EVENT_OneParam(AMGameStateInGame, FOnBossSpawnedEvent, AActor* /*BossMonster*/)
	FOnBossSpawnedEvent OnBossMonsterSet;
public:
	UFUNCTION()
	void OnRep_BossMonster() { OnBossMonsterSet.Broadcast(BossMonster); }
	AMonsterSpawner* GetMonsterSpawner() { return MonsterSpawner.Get(); }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMonsterSpawner> MonsterSpawnerClass = nullptr;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AMonsterSpawner> MonsterSpawner = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_BossMonster)
	AActor* BossMonster;

};

DECLARE_EVENT_OneParam(URoundComponent, FOnRoundChangedEvent, const FRound& /*Round*/);
DECLARE_EVENT(URoundComponent, FOnWaitNextRound);

UCLASS(Blueprintable)
class TESTGAME_API URoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URoundComponent();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool IsLastRound() const;

	// 웨이브
public:
	UFUNCTION(BlueprintPure)
	bool IsLastWave() const;
	UFUNCTION(BlueprintPure)
	bool IsLastWaveFor(int InWave) const;
	bool IsFinished() const;
	UFUNCTION(BlueprintPure)
	FRoundInfo GetRoundTableData(int32 InRound) const;
	const FRound& GetRoundWave() const { return RoundWaveData; }
	int32 GetRound() const { return RoundWaveData.Round; }
	int32 GetWave() const { return RoundWaveData.Wave; }
public:
	void TryNextRound(float Delay);
	UFUNCTION(BlueprintCallable)
	void NextRound();
	UFUNCTION(BlueprintCallable)
	void StartWave();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Wave(const FRound& InRound);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WaitNextRound();
public:
	FOnWaitNextRound& GetWaitNextRoundEvent() { return OnWaitNextRoundEvent; }
	FOnRoundChangedEvent& GetRoundChangeEvent() { return OnRoundChangedEvent; }
public:
	FOnWaitNextRound OnWaitNextRoundEvent;
	FOnRoundChangedEvent OnRoundChangedEvent;
	FOnRoundChangedEvent OnRoundAndWaveChangedEvent;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UDataTable* RoundTable;

	UPROPERTY(Replicated)
	FRound RoundWaveData;
	FTimerHandle NextRoundTimerHandle;
	FTimerHandle NextWaveTimerHandle;
	bool bAllRoundFinished = false;
};

UINTERFACE()
class TESTGAME_API URoundInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IRoundInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsClear();
};

UCLASS()
class TESTGAME_API ARoundReward : public AActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	virtual void SpawnReward();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> RewardClass = nullptr;
	TArray<FVector> RewardLocations;
};