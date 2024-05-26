// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGameStateInGame.h"
#include "GameFramework/PlayerState.h"

void AMGameStateInGame::BeginPlay()
{
	Super::BeginPlay();
}

void AMGameStateInGame::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGameStateInGame, RoundInfo);
}

void AMGameStateInGame::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (HasAuthority())
	{
		TryNextRound();
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

void AMGameStateInGame::TryNextRound()
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

void AMGameStateInGame::NextRount()
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
		RoundInfo = *NextRoundInfo;
		CurrentRoundName = NextRoundName;
		++Round;
		OnRep_RoundInfoChanged();
	}
}

void AMGameStateInGame::OnRep_RoundInfoChanged()
{
	RoundStartedDelegate.Broadcast(RoundInfo);
	RoundStartedDynamicDelegate.Broadcast();

	if (RoundInfo.StartCondition == ERoundStartCondition::Timer)
	{
		FTimerHandle Dummy;
		GetWorld()->GetTimerManager().SetTimer(Dummy, [this]() { TryNextRound(); }, RoundInfo.Timer, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Round %d"), Round);
}

bool AMGameStateInGame::IsAllMonsterDead()
{
	return false;
}
