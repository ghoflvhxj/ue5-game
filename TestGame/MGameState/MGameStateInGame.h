// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "MGameStateInGame.generated.h"

class APlayerState;

UENUM()
enum class ERoundStartCondition : uint8
{
	None,
	AllMonsterDead,
	Timer
};

USTRUCT(BlueprintType)
struct FRoundInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoundStartCondition StartCondition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Timer;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<AMCharacter>> MonsterClassArray;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpawnNum;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStartedDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDynamicDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoundStartedDelegate, FRoundInfo);

UCLASS()
class TESTGAME_API AMGameStateInGame : public AGameState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;
	virtual void OnRep_MatchState() override;
	DECLARE_EVENT_OneParam(AMGameStateInGame, FOnMatchStateChangedEvent, FName);
	FOnMatchStateChangedEvent OnMatchStateChanegdEvent;
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

	// 라운드
public:
	void TryNextRound();
private:
	void NextRount();
	UFUNCTION()
	void OnRep_RoundInfoChanged();
public:
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnRoundStartedDynamicDelegate RoundStartedDynamicDelegate;
	FOnRoundStartedDelegate RoundStartedDelegate;
public:
	int32 GetRound() { return Round; }
	const FRoundInfo& GetRoundInfo() { return RoundInfo; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* RoundTable;
	FName CurrentRoundName = NAME_None;
private:
	int32 Round;
	UPROPERTY(ReplicatedUsing = OnRep_RoundInfoChanged)
	FRoundInfo RoundInfo;

private:
	bool IsAllMonsterDead();

};
