// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGameStateInGame.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "TestGame/MSpawner/MonsterSpawner.h"

DECLARE_LOG_CATEGORY_CLASS(LogRound, Log, Log);

AMGameStateInGame::AMGameStateInGame()
{
	RoundComponent = CreateDefaultSubobject<URoundComponent>(TEXT("RoundComponent"));
}

void AMGameStateInGame::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		MonsterSpawner = GetWorld()->SpawnActor<AMonsterSpawner>(MonsterSpawnerClass);
		if (MonsterSpawner.IsValid())
		{
			MonsterSpawner->OnBossSpawnedEvent.AddWeakLambda(this, [this](AActor* InBossMonster) {
				BossMonster = InBossMonster;
				OnRep_BossMonster();
			});
		}
	}
}

void AMGameStateInGame::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGameStateInGame, BossMonster);
}

void AMGameStateInGame::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (HasAuthority() && IsValid(RoundComponent))
	{
		RoundComponent->TryNextRound(nullptr);
	}
}

void AMGameStateInGame::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	OnMatchEndEvent.Broadcast();
}

void AMGameStateInGame::AddDeadPlayer(APlayerState* DeadPlayerState)
{
	DeadPlayerArray.AddUnique(DeadPlayerState);
}

void AMGameStateInGame::RemoveDeadPlayer(APlayerState* DeadPlayerState)
{
	DeadPlayerArray.Remove(DeadPlayerState);
}

bool AMGameStateInGame::IsAllPlayerDead()
{
	return PlayerArray.Num() == DeadPlayerArray.Num();
}

bool AMGameStateInGame::IsRevivalable()
{
	return PlayerLife > 0;
}

bool AMGameStateInGame::RevivePlayer(APlayerState* PlayerState)
{
	if (IsRevivalable() == false)
	{
		return false;
	}

	--PlayerLife;
	RemoveDeadPlayer(PlayerState);
	return true;
}

void AMGameStateInGame::Multicast_GameOver_Implementation()
{
	GameOverDynamicDelegate.Broadcast();
}

URoundComponent::URoundComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void URoundComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URoundComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(URoundComponent, Round, COND_InitialOnly);
}

bool URoundComponent::IsLastRound() const
{
	if (IsValid(RoundTable))
	{
		TArray<FRoundInfo*> AllRoundInfo;
		RoundTable->GetAllRows(TEXT("RoundTable"), AllRoundInfo);

		return AllRoundInfo.Last()->Round == Round;
	}

	return false;
}

FRoundInfo URoundComponent::GetRoundInfo(int32 InRound) const
{
	if (IsValid(RoundTable))
	{
		if (FRoundInfo* FoundRoundInfo = RoundTable->FindRow<FRoundInfo>(FName(FString::FromInt(InRound)), TEXT("RoundTable")))
		{
			return *FoundRoundInfo;
		}
	}

	return FRoundInfo();
}

void URoundComponent::TryNextRound(AActor* Rounder)
{
	if (IsLastRound() && IsLastWave())
	{
		bAllRoundFinished = true;
		return;
	}

	++Round;
	Wave = 1;

	StartWave();
}

void URoundComponent::StartWave()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	// 인위적(치트 등)으로 다음 웨이브가 실행됬다면, 타이머는 해제시킴
	if (NextWaveTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(NextWaveTimerHandle);
	}

	Multicast_Wave(Round, Wave);

	if (IsLastWave() == false)
	{
		const FRoundInfo& RoundInfo = GetRoundInfo(Round);
		if (RoundInfo.IsValid())
		{
			World->GetTimerManager().SetTimer(NextWaveTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
				StartWave();
			}), RoundInfo.WaveIntervalBase, false);
		}

		++Wave;
	}
}

bool URoundComponent::IsLastWave() const
{
	return IsLastWave(Wave);
}

bool URoundComponent::IsLastWave(int InWave) const
{
	const FRoundInfo& RoundInfo = GetRoundInfo(Round);
	if (RoundInfo.IsValid())
	{
		return InWave == RoundInfo.TotalWave;
	}

	return false;
}

bool URoundComponent::IsFinished() const
{
	return bAllRoundFinished;
}

void URoundComponent::Multicast_Wave_Implementation(int32 InRound, int32 InWave)
{
	const FRoundInfo& RoundInfo = GetRoundInfo(InRound);
	if (RoundInfo.IsValid() == false)
	{
		return;
	}

	if (IsNetSimulating())
	{
		Round = InRound;
		Wave = InWave;
	}

	OnWaveChangedEvent.Broadcast(Round, RoundInfo, Wave);
	UE_LOG(LogRound, Log, TEXT("RoundInfo Updated Round:%d"), RoundInfo.Round);
}
