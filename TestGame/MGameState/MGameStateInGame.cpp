// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGameStateInGame.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include "MGameInstance.h"
#include "MyPlayerState.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MItem/ItemBase.h"
#include "TestGame/MSpawner/MonsterSpawner.h"
#include "TestGame/MPlayerController/MPlayerController.h"

DECLARE_LOG_CATEGORY_CLASS(LogRound, Log, Log);
DECLARE_LOG_CATEGORY_CLASS(LogReward, Log, Log);

AMGameStateInGame::AMGameStateInGame()
{
	PrimaryActorTick.bCanEverTick = true;

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

void AMGameStateInGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid World."));
		return;
	}

	for (const TPair<FName, float> Pair : MPCParamToStart)
	{
		FName MPCParamName = Pair.Key;
		MPCParamToElpasedTime.FindOrAdd(MPCParamName) = World->GetTimeSeconds() - Pair.Value;

		SetMPCParamValue(MPCParamName, MPCParamToElpasedTime[MPCParamName]);
	}

	//if (HasAuthority() == false)
	//{
	//	if (bMatchEndSuccess == false && HasMatchEnded())
	//	{
	//		HandleMatchHasEnded();
	//	}
	//}
}

void AMGameStateInGame::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGameStateInGame, BossMonster);
}

void AMGameStateInGame::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (IsValid(RoundComponent))
	{
		if (HasAuthority())
		{
			RoundComponent->TryNextRound(3.f);
		}
	}
	
	if (AMCharacter* MyCharacter = Cast<AMCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = MyCharacter->GetAbilitySystemComponent())
		{
			AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &AMGameStateInGame::ApplyItemEvent);
			AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &AMGameStateInGame::RemoveItemEvent);
		}
	}
}

void AMGameStateInGame::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	bool bMatchEndSuccess = false;
	if (IsAllPlayerDead())
	{
		OnMatchEndEvent.Broadcast(EEndMatchReason::Fail);
		bMatchEndSuccess = true;
	}
	else if(RoundComponent->IsFinished())
	{
		OnMatchEndEvent.Broadcast(EEndMatchReason::Clear);
		bMatchEndSuccess = true;
	}

	if (EndMatchRetryTimer.IsValid() == false && bMatchEndSuccess == false)
	{
		GetWorldTimerManager().SetTimer(EndMatchRetryTimer, this, &AMGameStateInGame::HandleMatchHasEnded, 1.f, true);
	}

	if (bMatchEndSuccess)
	{
		GetWorldTimerManager().ClearTimer(EndMatchRetryTimer);
		EndMatchRetryTimer.Invalidate();
	}
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
	if (PlayerArray.Num() == 0)
	{
		return true;
	}

	for (APlayerState* PlayerState : PlayerArray)
	{
		AMPlayerState* MPlayerState = Cast<AMPlayerState>(PlayerState);
		if (IsValid(MPlayerState) == false)
		{
			continue;
		}

		if (MPlayerState->IsDead() == false)
		{
			return false;
		}
	}

	return true;
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

int32 AMGameStateInGame::GetAlivePlayerNum()
{
	int32 AliveNum = 0;

	for (APlayerState* PlayerState : PlayerArray)
	{
		AMPlayerState* MPlayerState = Cast<AMPlayerState>(PlayerState);
		if (IsValid(MPlayerState) == false)
		{
			continue;
		}

		if (MPlayerState->IsDead() == false)
		{
			++AliveNum;
		}
	}

	return AliveNum;
}

const TSet<AActor*>& AMGameStateInGame::GetMonsters()
{
	if (MonsterSpawner.IsValid())
	{
		return MonsterSpawner->GetSpawnedActors();
	}

	static const TSet<AActor*> Empty;
	return Empty;
}

void AMGameStateInGame::RegistMPCParam(FName InParamName)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid World."));
		return;
	}

	MPCParamToStart.FindOrAdd(InParamName) = World->GetTimeSeconds();

	//if (InParamName == TEXT("Test"))
	//{
	//	RoundComponent->Pause();
	//}
}

void AMGameStateInGame::UnregistMPCParam(FName InParamName)
{
	MPCParamToStart.Remove(InParamName);
	MPCParamToElpasedTime.Remove(InParamName);
	
	SetMPCParamValue(InParamName, 0.f);
}

void AMGameStateInGame::SetMPCParamValue(FName InParamName, float InValue)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid World."));
		return;
	}

	UMaterialParameterCollectionInstance* MaterialParameterCollectionInst = World->GetParameterCollectionInstance(MaterialParameterCollection);
	if (IsValid(MaterialParameterCollectionInst) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid UMaterialParameterCollectionInstance."));
		return;
	}

	if (MaterialParameterCollectionInst->SetScalarParameterValue(InParamName, InValue) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameter [%s]."), *InParamName.ToString());
	}
}

void AMGameStateInGame::ApplyItemEvent(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InEffectSpec, FActiveGameplayEffectHandle InActiveEffectHandle)
{
	FGameplayTagContainer GETagContainer;
	InEffectSpec.GetAllAssetTags(GETagContainer);

	if (GETagContainer.HasTag(FGameplayTag::RequestGameplayTag("Event.TimeStop")))
	{
		if (IsValid(RoundComponent))
		{
			RoundComponent->Pause();
		}
		
		if (MonsterSpawner.IsValid())
		{
			MonsterSpawner->SetActorTickEnabled(false);
		}
	}
}

void AMGameStateInGame::RemoveItemEvent(const FActiveGameplayEffect& InActiveGameplayEffect)
{
	FGameplayTagContainer GETagContainer;
	InActiveGameplayEffect.Spec.GetAllAssetTags(GETagContainer);

	if (GETagContainer.HasTag(FGameplayTag::RequestGameplayTag("Event.TimeStop")))
	{
		if (IsValid(RoundComponent))
		{
			RoundComponent->Resume();
		}
		if (MonsterSpawner.IsValid())
		{
			MonsterSpawner->SetActorTickEnabled(true);
		}
	}
}

void AMGameStateInGame::ChangeBGM(USoundBase* InSound)
{
	//if (IsValid(AudioComponent))
	//{
	//	AudioComponent->SetPaused(true);
	//}
	AudioComponent = UGameplayStatics::SpawnSound2D(this, InSound, 1.f, 1.f, 0.f, BGMConcurrency);
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

void URoundComponent::Pause()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	World->GetTimerManager().PauseTimer(NextWaveTimerHandle);
	World->GetTimerManager().PauseTimer(NextRoundTimerHandle);

	bPause = true;
	OnRoundPausedEvent.Broadcast(true);
}

void URoundComponent::Resume()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	World->GetTimerManager().UnPauseTimer(NextWaveTimerHandle);
	World->GetTimerManager().UnPauseTimer(NextRoundTimerHandle);

	bPause = false;
	OnRoundPausedEvent.Broadcast(false);
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
	UE_LOG(LogTemp, Warning, TEXT("HandleMatchHasStarted World validation: %d"), (int)IsValid(World));
	if (IsValid(World) == false)
	{
		return;
	}

	if (IsLastRound() && IsLastWave())
	{
		Multicast_AllRoundFinished();
		return;
	}

	if (Delay <= 0.f)
	{
		NextRound();
	}
	else
	{
		Multicast_WaitNextRound();
		World->GetTimerManager().SetTimer(NextRoundTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			NextRound();
		}), Delay, false);
	}
}

void URoundComponent::NextRound()
{
	++RoundWaveData.Round;
	RoundWaveData.Wave = 0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NextRoundTimerHandle);
		NextRoundTimerHandle.Invalidate();
	}

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

void URoundComponent::FinishRound()
{
	Multicast_RoundFinish();
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

void URoundComponent::Multicast_RoundFinish_Implementation()
{
	OnRoundFinishedEvent.Broadcast();
}

void URoundComponent::Multicast_AllRoundFinished_Implementation()
{
	bAllRoundFinished = true;
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

void ARoundReceiveActor::BeginPlay()
{
	Super::BeginPlay();
	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(this))
	{
		if (URoundComponent* RoundComponent = GameState->GetComponentByClass<URoundComponent>())
		{
			RoundComponent->GetRoundFinishedEvenet().AddWeakLambda(this, [this, RoundComponent]() {
				if (RoundComponent->GetRound() == BoundRound.Round)
				{
					ReceiveRoundFinish();
				}
			});
			RoundComponent->GetWaitNextRoundEvent().AddWeakLambda(this, [this, RoundComponent]() {
				if (RoundComponent->GetRound() == BoundRound.Round)
				{
					ReceiveWaitNextRound();
				}
			});

			RoundComponent->GetRoundChangedEvent().AddUObject(this, &ARoundReceiveActor::ReceiveRound);
		}
	}
}

void ARoundReceiveActor::ReceiveWaitNextRound_Implementation()
{

}

void ARoundReceiveActor::ReceiveRoundFinish_Implementation()
{

}

void ARoundReceiveActor::ReceiveRound_Implementation(const FRound& InRound)
{

}
