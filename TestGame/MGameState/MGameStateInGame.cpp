// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGameStateInGame.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "TestGame/MSpawner/MonsterSpawner.h"
#include "TestGame/MPlayerController/MPlayerController.h"

DECLARE_LOG_CATEGORY_CLASS(LogRound, Log, Log);
DECLARE_LOG_CATEGORY_CLASS(LogReward, Log, Log);

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
		GetWorldTimerManager().SetTimer(GameStartTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) 
			{
				if (AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(It->Get()))
				{
					if (PlayerController->IsReady() == false)
					{
						return;
					}
				}
			}

			RoundComponent->TryNextRound(3.f);
			GetWorldTimerManager().ClearTimer(GameStartTimerHandle);

		}), 1.f, true);
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

	DOREPLIFETIME_CONDITION(URoundComponent, RoundWaveData, COND_InitialOnly);
}

bool URoundComponent::IsLastRound() const
{
	if (IsValid(RoundTable))
	{
		TArray<FRoundInfo*> AllRoundInfo;
		RoundTable->GetAllRows(TEXT("RoundTable"), AllRoundInfo);

		return AllRoundInfo.Last()->Round == RoundWaveData.Round;
	}

	return false;
}

FRoundInfo URoundComponent::GetRoundTableData(int32 InRound) const
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

void URoundComponent::SetRoundWave(const FRound& InRound)
{
	if (IsNetSimulating())
	{
		return;
	}

	if (RoundWaveData != InRound)
	{
		RoundWaveData = InRound;
		Multicast_RoundWave(RoundWaveData);
	}
}

void URoundComponent::TryNextRound(float Delay)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	if (IsLastRound() && IsLastWave())
	{
		bAllRoundFinished = true;
		return;
	}

	if (Delay <= 0.f)
	{
		NextRound();
	}
	else
	{
		World->GetTimerManager().SetTimer(NextRoundTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			NextRound();
		}), Delay, false);

		Multicast_WaitNextRound();
	}
}

void URoundComponent::NextRound()
{
	++RoundWaveData.Round;
	RoundWaveData.Wave = 0;

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

	const FRoundInfo& RoundInfo = GetRoundTableData(RoundWaveData.Round);
	if (RoundInfo.IsValid() && IsLastWave() == false)
	{
		World->GetTimerManager().SetTimer(NextWaveTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			StartWave();
		}), RoundInfo.WaveIntervalBase, false);

		++RoundWaveData.Wave;
		RoundWaveData.NextWaveTime = World->GetGameState()->GetServerWorldTimeSeconds() + RoundInfo.WaveIntervalBase;

		Multicast_RoundWave(RoundWaveData);
	}
}

bool URoundComponent::IsLastWave() const
{
	return IsLastWaveFor(RoundWaveData.Wave);
}

bool URoundComponent::IsLastWaveFor(int InWave) const
{
	const FRoundInfo& RoundInfo = GetRoundTableData(RoundWaveData.Round);
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

void URoundComponent::Multicast_RoundWave_Implementation(const FRound& InRound)
{
	const FRoundInfo& RoundInfo = GetRoundTableData(InRound.Round);
	if (RoundInfo.IsValid() == false)
	{
		return;
	}

	if (IsNetSimulating())
	{
		RoundWaveData = InRound;
	}

	if (RoundWaveData.Wave == 1)
	{
		OnRoundChangedEvent.Broadcast(RoundWaveData);
	}

	OnRoundAndWaveChangedEvent.Broadcast(RoundWaveData);
	UE_LOG(LogRound, Log, TEXT("Wave Updated %d-%d"), RoundWaveData.Round, RoundWaveData.Wave);
}

void URoundComponent::Multicast_WaitNextRound_Implementation()
{
	OnWaitNextRoundEvent.Broadcast();
}

void ARoundReward::BeginPlay()
{
	Super::BeginPlay();
	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(this))
	{
		if (URoundComponent* RoundComponent = GameState->GetComponentByClass<URoundComponent>())
		{
			if (HasAuthority())
			{
				RoundComponent->GetWaitNextRoundEvent().AddUObject(this, &ARoundReward::SpawnReward);
			}
		}
	}
}

void ARoundReward::SpawnReward()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	if (IsValid(RewardClass) == false)
	{
		UE_LOG(LogReward, Warning, TEXT("Reward class invalid."));
		return;
	}

	if (RewardLocations.Num() == 0)
	{
		UE_LOG(LogReward, Log, TEXT("Reward location is empty. Use this actor location."));
		RewardLocations.Add(GetActorLocation());
	}

	FTransform Transform;
	Transform.SetLocation(RewardLocations[0]);
	if (AActor* Reward = World->SpawnActorDeferred<AActor>(RewardClass, Transform))
	{
		UGameplayStatics::FinishSpawningActor(Reward, Transform);
	}
}
