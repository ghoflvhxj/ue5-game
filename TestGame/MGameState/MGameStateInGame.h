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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStartedDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDynamicDelegate);
DECLARE_EVENT(AMGameStateInGame, FOnMatchEndEvent);

UCLASS()
class TESTGAME_API AMGameStateInGame : public AGameState
{
	GENERATED_BODY()

public:
	AMGameStateInGame();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
public:
	FOnMatchEndEvent OnMatchEndEvent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URoundComponent* RoundComponent = nullptr;

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
	FOnBossSpawnedEvent OnBossSpawnedEvent;
public:
	AMonsterSpawner* GetMonsterSpawner() { return Cast<AMonsterSpawner>(MonsterSpawner.Get()); }
	UFUNCTION(Reliable, NetMulticast)
	void Multicast_BossSpawned(AActor* Boss);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMonsterSpawner> MonsterSpawnerClass = nullptr;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> MonsterSpawner = nullptr;
};

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
	bool IsLastWave() const;
	bool IsLastWave(int InWave) const;
	bool IsFinished() const;
	UFUNCTION(BlueprintPure)
	FRoundInfo GetRoundInfo(int32 InRound) const;
public:
	void TryNextRound(AActor* Rounder);
	UFUNCTION(BlueprintCallable)
	void StartWave();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Wave(int32 InRound, int32 InWave);
public:
	DECLARE_EVENT_ThreeParams(URoundComponent, FOnRoundChangedEvent, int /*Round*/, const FRoundInfo&, int /*Wave*/);
	FOnRoundChangedEvent OnWaveChangedEvent;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UDataTable* RoundTable;
	FName CurrentWaveName = NAME_None;
	UPROPERTY(Replicated)
	int32 Round = 0;
	UPROPERTY()
	int32 Wave = 0;
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