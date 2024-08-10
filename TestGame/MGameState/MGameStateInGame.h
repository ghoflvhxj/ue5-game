// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "MGameStateInGame.generated.h"

class APlayerState;

UENUM()
enum class EClearType : uint8
{
	None,
	AllMonsterDead,
	Timer
};

USTRUCT(BlueprintType)
struct FRoundInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Round;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EClearType ClearType;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStartedDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDynamicDelegate);

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
	virtual void OnRep_MatchState() override;
	DECLARE_EVENT_OneParam(AMGameStateInGame, FOnMatchStateChangedEvent, FName);
	FOnMatchStateChangedEvent OnMatchStateChanegdEvent;

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
};

UCLASS(Blueprintable)
class TESTGAME_API URoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 라운드
public:
	bool IsRoundStarted();
	const FRoundInfo& GetRoundInfo() { return RoundInfo; }
public:
	void TryNextRound();
	void NextRount();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RoundInfo(const FRoundInfo& InRoundInfo);
public:
	DECLARE_EVENT_OneParam(URoundComponent, FOnRoundChangedEvent, const FRoundInfo&);
	FOnRoundChangedEvent RoundChangedEvent;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UDataTable* RoundTable;
	UPROPERTY(Replicated)
	FRoundInfo RoundInfo;
	FName CurrentRoundName = NAME_None;
};