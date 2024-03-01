// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterSpawner.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MCharacterEnum.h"
#include "TestGame/MGameState/MGameStateInGame.h"

AActor* ASpawner::Spawn()
{
	UWorld* World = GetWorld();
	check(IsValid(World));

	FSpawnInfo SpawnInfo;
	if (MakeSpawnInfo(SpawnInfo) == false)
	{
		return nullptr;
	}

	if (IsValid(SpawnInfo.SpawnClass))
	{
		FTransform SpawnTransform;
		if (MakeSpawnTransform(SpawnTransform))
		{
			return World->SpawnActor<AActor>(SpawnInfo.SpawnClass, SpawnTransform);
		}
	}

	return nullptr;
}

bool ASpawner::MakeSpawnInfo(FSpawnInfo& SpawnInfo)
{
	return false;
}

bool ASpawner::MakeSpawnTransform(FTransform& Transfrom)
{
	return false;
}

void AMonsterSpawner::BeginPlay()
{
	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
	check(GameState);

	GameState->RoundStartedDelegate.AddWeakLambda(this, [this, GameState](FRoundInfo RoundInfo) {
		for (int i = 0; i < RoundInfo.SpawnNum; ++i)
		{
			AMCharacter* SpawnedMonster = Cast<AMCharacter>(Spawn());
			if (IsValid(SpawnedMonster) == false)
			{
				continue;
			}

			SpawnedMonster->AddVitalityChangedDelegate(this, [this, SpawnedMonster, RoundInfo, GameState](uint8 OldValue, uint8 NewValue) {
				if (NewValue == static_cast<uint8>(ECharacterVitalityState::Die))
				{
					SpawnedMonsters.Remove(SpawnedMonster);
				}

				if (RoundInfo.StartCondition == ERoundStartCondition::AllMonsterDead)
				{
					if (SpawnedMonsters.Num() == 0)
					{
						GameState->TryNextRound();
					}
				}
			});
		}

		if (RoundInfo.StartCondition == ERoundStartCondition::AllMonsterDead)
		{

		}
	});
}

//AActor* AMonsterSpawner::Spawn()
//{
//	AMCharacter* SpawnedMonster = Cast<AMCharacter>(Super::Spawn());
//
//
//	return SpawnedMonster;
//}

bool AMonsterSpawner::MakeSpawnInfo(FSpawnInfo& SpawnInfo)
{
	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
	check(GameState);

	//FName RowName = FName(FString::Printf(TEXT("%d"), GameState->GetRound()));
	//if (FSpawnInfo* TableRow = SpawnTable->FindRow<FSpawnInfo>(RowName, nullptr))
	//{
	//	SpawnInfo = *TableRow;
	//	return true;
	//}

	FRoundInfo RoundInfo = GameState->GetRoundInfo();
	if (RoundInfo.MonsterClassArray[0] != nullptr)
	{
		SpawnInfo.SpawnClass = RoundInfo.MonsterClassArray[0];
		return true;
	}

	return false;
}

bool AMonsterSpawner::MakeSpawnTransform(FTransform& Transform)
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		Transform = Pawn->GetTransform();
	}
	else
	{
		float rnd = (2.f * FMath::FRand()) - 1.f;
		float rnd2 = (2.f * FMath::FRand()) - 1.f;
		Transform.SetLocation(FVector(1000.f * rnd, 1000.f * rnd2, 0.f));
	}


	return true;
}
