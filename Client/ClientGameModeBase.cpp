// Copyright Epic Games, Inc. All Rights Reserved.


#include "ClientGameModeBase.h"
#include "OnlineSubsystem.h"

void AClientGameModeBase::LeaderboardTest()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	IOnlineLeaderboardsPtr Leaderboards = OnlineSubsystem->GetLeaderboardsInterface();
	if (Leaderboards == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("리더보드 인터페이스 없음"));
		return;
	}
	\
		ReadObject = MakeShareable(new FOnlineLeaderboardRead());
	FOnlineLeaderboardReadRef ReadObjectRef = ReadObject.ToSharedRef();
	ReadObjectRef->LeaderboardName = FName(TEXT("GM0002Single"));

	LeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &AClientGameModeBase::PrintLeaderboard);
	FDelegateHandle LeaderboardReadCompleteDelegateHandle = Leaderboards->AddOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegate);
	Leaderboards->ReadLeaderboardsAroundRank(5, 3, ReadObjectRef);
}

void AClientGameModeBase::PrintLeaderboard(bool b)
{
	UE_LOG(LogTemp, Warning, TEXT("리더보드 프린트"));

	for (int32 RowIdx = 0; RowIdx < ReadObject->Rows.Num(); ++RowIdx)
	{
		const FOnlineStatsRow& StatsRow = ReadObject->Rows[RowIdx];
		UE_LOG_ONLINE_LEADERBOARD(Log, TEXT("   Leaderboard stats for: Nickname = %s, Rank = %d"), *StatsRow.NickName, StatsRow.Rank);

		for (FStatsColumnArray::TConstIterator It(StatsRow.Columns); It; ++It)
		{
			UE_LOG_ONLINE_LEADERBOARD(Log, TEXT("     %s = %s"), *It.Key().ToString(), *It.Value().ToString());
		}
	}
}
