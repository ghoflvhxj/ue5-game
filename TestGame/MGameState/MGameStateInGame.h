// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AbilitySystemComponent.h"
#include "MGameStateInGame.generated.h"

class APlayerState;
class AMonsterSpawner;


UENUM(BlueprintType)
enum class EEndMatchReason : uint8
{
	None,
	Fail,
	Clear
};

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

	bool operator==(const FRound& Rhs)
	{
		return Round == Rhs.Round && Wave == Rhs.Wave;
	}
	bool operator!=(const FRound& Rhs)
	{
		return *this == Rhs;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStartedDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDynamicDelegate);
DECLARE_EVENT_OneParam(AMGameStateInGame, FOnMatchEndEvent, EEndMatchReason);

UCLASS()
class TESTGAME_API AMGameStateInGame : public AGameState
{
	GENERATED_BODY()

public:
	AMGameStateInGame();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URoundComponent* RoundComponent = nullptr;
	UAudioComponent* AudioComponent = nullptr;
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
public:
	FOnMatchEndEvent OnMatchEndEvent;

protected:
	// 복제가 덜 되어서 클라에서 EndMatch처리가 안됬음을 나타냄
	//bool bMatchEndSuccess = false;
	// 복제가 덜 되어서 클라에서 EndMatch처리가 안된 경우 지속적으로 EndMatch를 시도하기 위한 타이머
	FTimerHandle EndMatchRetryTimer;

	// 부활
public:
	bool IsRevivalable();
	bool RevivePlayer(APlayerState* PlayerState);
	UFUNCTION(BlueprintPure)
	int32 GetAlivePlayerNum();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PlayerLife;

	// 게임 오버
public:
	void AddDeadPlayer(APlayerState* DeadPlayerState);
	void RemoveDeadPlayer(APlayerState* DeadPlayerState);
	bool IsAllPlayerDead();
	
	TArray<APlayerState*> DeadPlayerArray;

public:
	FOnGameOverDynamicDelegate GameOverDynamicDelegate;

/* 몬스터 관련 */
public:
	DECLARE_EVENT_OneParam(AMGameStateInGame, FOnBossSpawnedEvent, AActor* /*BossMonster*/)
	FOnBossSpawnedEvent OnBossMonsterSet;
public:
	UFUNCTION()
	void OnRep_BossMonster() { OnBossMonsterSet.Broadcast(BossMonster); }
	AMonsterSpawner* GetMonsterSpawner() { return MonsterSpawner.Get(); }
	const TSet<AActor*>& GetMonsters();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMonsterSpawner> MonsterSpawnerClass = nullptr;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AMonsterSpawner> MonsterSpawner = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_BossMonster)
	AActor* BossMonster = nullptr;

/* MPC */
public:
	void RegistMPCParam(FName InParamName);
	void UnregistMPCParam(FName InParamName);
	void SetMPCParamValue(FName InParamName, float InValue);
protected:
	TMap<FName, float> MPCParamToElpasedTime;
	TMap<FName, float> MPCParamToStart;
	UPROPERTY(EditDefaultsOnly)
	UMaterialParameterCollection* MaterialParameterCollection = nullptr;

/* 아이템 */
public:
	void ApplyItemEvent(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InEffectSpec, FActiveGameplayEffectHandle InActiveEffectHandle);
	void RemoveItemEvent(const FActiveGameplayEffect& InActiveGameplayEffect);

/* 사운드 */
public:
	UFUNCTION(BlueprintCallable)
	void ChangeBGM(USoundBase* InSound);
protected:
	UPROPERTY(EditDefaultsOnly)
	USoundConcurrency* BGMConcurrency = nullptr;
};

DECLARE_EVENT_OneParam(URoundComponent, FOnRoundChangedEvent, const FRound& /*Round*/);
DECLARE_EVENT_OneParam(URoundComponent, FOnRoundPausedEvent, bool /* bPaused */)
DECLARE_EVENT(URoundComponent, FRoundEvent);

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

public:
	UFUNCTION(BlueprintPure)
	bool IsPaused() const { return bPause; }
	FOnRoundPausedEvent& GetRoundPausedEvent() { return OnRoundPausedEvent; }
public:
	void Pause();
	void Resume();
protected:
	bool bPause = false;
	FOnRoundPausedEvent OnRoundPausedEvent;

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
	UFUNCTION(BlueprintPure)
	float GetWaveReaminigTime() const;
public:
	void SetRoundWave(const FRound& InRound);
	UFUNCTION(BlueprintCallable)
	void TryNextRound(float Delay);
	UFUNCTION(BlueprintCallable)
	void NextRound();
	UFUNCTION(BlueprintCallable)
	void StartWave();
	UFUNCTION(BlueprintCallable)
	void FinishRound();
public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RoundWave(const FRound& InRound);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WaitNextRound();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RoundFinish();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AllRoundFinished();
public:
	FRoundEvent& GetRoundFinishedEvenet() { return OnRoundFinishedEvent; }
	FRoundEvent& GetWaitNextRoundEvent() { return OnWaitNextRoundEvent; }
	FOnRoundChangedEvent& GetRoundChangedEvent() { return OnRoundChangedEvent; }
	FOnRoundChangedEvent& GetRoundAndWaveChangedEvent() { return OnRoundAndWaveChangedEvent; }
protected:
	FRoundEvent OnRoundFinishedEvent;
	FRoundEvent OnWaitNextRoundEvent;
	FOnRoundChangedEvent OnRoundChangedEvent;
	FOnRoundChangedEvent OnRoundAndWaveChangedEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UDataTable* RoundTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
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

UCLASS()
class TESTGAME_API ARoundReceiveActor : public AActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveWaitNextRound();
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveRoundFinish();
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveRound(const FRound& InRound);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRound BoundRound;
};