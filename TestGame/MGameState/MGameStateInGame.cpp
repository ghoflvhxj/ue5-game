// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGameStateInGame.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"

DECLARE_LOG_CATEGORY_CLASS(LogRound, Log, Log);

AMGameStateInGame::AMGameStateInGame()
{
	RoundComponent = CreateDefaultSubobject<URoundComponent>(TEXT("RoundComponent"));
}

void AMGameStateInGame::BeginPlay()
{
	Super::BeginPlay();
}

void AMGameStateInGame::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMGameStateInGame::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (HasAuthority() && IsValid(RoundComponent))
	{
		RoundComponent->TryNextRound();
	}
}

void AMGameStateInGame::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	OnMatchStateChanegdEvent.Broadcast(GetMatchState());
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

void URoundComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AGameStateBase* GameState = Cast<AGameStateBase>(GetOwner()))
	{
		//FGame
	}
}

void URoundComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(URoundComponent, RoundInfo, COND_InitialOnly);
}

bool URoundComponent::IsRoundStarted()
{
	return CurrentRoundName != NAME_None;
}

void URoundComponent::TryNextRound()
{
	//switch (RountInfo.StartCondition)
	//{
	//	case ERoundStartCondition::None:
	//	{

	//	}
	//	break;
	//	case ERoundStartCondition::AllMonsterDead:
	//	{
	//	}
	//	break;
	//}

	NextRount();
}

void URoundComponent::NextRount()
{
	if (IsValid(RoundTable) == false)
	{
		return;
	}

	TArray<FName> RoundRowNames = RoundTable->GetRowNames();

	if (bool bIsLastRound = CurrentRoundName == RoundRowNames.Last())
	{
		return;
	}

	FName NextRoundName = NAME_None;
	if (CurrentRoundName == NAME_None)
	{
		NextRoundName = RoundTable->GetRowNames()[0];
	}
	else
	{
		int32 RoundIndex = 0;
		if (RoundRowNames.Find(CurrentRoundName, RoundIndex) && RoundRowNames.IsValidIndex(RoundIndex + 1))
		{
			NextRoundName = RoundRowNames[RoundIndex + 1];
		}
	}

	if (FRoundInfo* NextRoundInfo = RoundTable->FindRow<FRoundInfo>(NextRoundName, nullptr))
	{
		Multicast_RoundInfo(*NextRoundInfo);
		CurrentRoundName = NextRoundName;
	}

	UE_LOG(LogRound, Log, TEXT("%s CurrentRound:%s"), *FString(__FUNCTION__), *CurrentRoundName.ToString());
}

void URoundComponent::Multicast_RoundInfo_Implementation(const FRoundInfo& InRoundInfo)
{
	RoundInfo = InRoundInfo;
	RoundChangedEvent.Broadcast(RoundInfo);

	UE_LOG(LogRound, Log, TEXT("RoundInfo Updated Round:%d"), RoundInfo.Round);
}
