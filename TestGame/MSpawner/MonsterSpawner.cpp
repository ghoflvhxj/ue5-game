// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterSpawner.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MCharacterEnum.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "Kismet/KismetMathLibrary.h"

#include "NavigationSystem.h"

DECLARE_LOG_CATEGORY_CLASS(Log_Spawner, Log, Log);

void ASpawner::Spawn(const FSpawnInfo& InSpawnInfo)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	if (bActivated == false)
	{
		return;
	}

	if (InSpawnInfo.IsValid() == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("스폰 정보가 유효하지 않음"));
		return;
	}
	
	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	switch (InSpawnInfo.SpawnType)
	{
		case ESpawnType::Fixed:
		{
			for (auto ActorClassToSpawnNum : InSpawnInfo.SpawneeClassMap)
			{
				int32 SpawnNum = static_cast<int32>(ActorClassToSpawnNum.Value);
				for (int32 SpawnCounter = 0; SpawnCounter < SpawnNum; ++SpawnCounter)
				{
					if (AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClassToSpawnNum.Key, GetSpawnTransform(), SpawnParam))
					{
						SpawnedActors.Add(SpawnedActor);
						OnSpawned(SpawnedActor);
					}
				}
			}
		}
		break;
		case ESpawnType::Probable:
		{
			for (int32 SpawnCounter = 0; SpawnCounter < 10; ++SpawnCounter)
			{
				float Test = FMath::FRand() * 100.f;
				for (auto ActorClassToSpawnNum : InSpawnInfo.SpawneeClassMap)
				{
					Test -= ActorClassToSpawnNum.Value;
					if (Test < 0.f)
					{
						if (AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClassToSpawnNum.Key, GetSpawnTransform(), SpawnParam))
						{
							SpawnedActors.Add(SpawnedActor);
							OnSpawned(SpawnedActor);
						}
						break;
					}
				}
			}
		}
		break;
		default:
		{

		}
		break;
	}
}

FTransform ASpawner::GetSpawnTransform()
{
	FTransform Transform = GetActorTransform();
	Transform.SetScale3D(FVector::OneVector);

	return Transform;
}

void ARoundSpanwer::BeginPlay()
{
	Super::BeginPlay();

	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
	check(GameState);

	if (HasAuthority())
	{
		if (URoundComponent* RoundComponent = GameState->GetComponentByClass<URoundComponent>())
		{
			RoundComponent->OnRoundChangedEvent.AddUObject(this, &ARoundSpanwer::SpawnUsingRoundInfo);
		}
	}
}

void AMonsterSpawner::SpawnUsingRoundInfo(const FRoundInfo& InRoundInfo)
{
	if (IsValid(MonsterSpawnTable) == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("%s 몬스터 스폰 테이블이 유효하지 않음"), *FString(__FUNCTION__));
		return;
	}

	FName CurrentRoundName = FName(FString::FromInt(InRoundInfo.Round));
	if (FSpawnInfo* SpawInfo = MonsterSpawnTable->FindRow<FSpawnInfo>(CurrentRoundName, TEXT("MonsterSpawnTable")))
	{
		Spawn(*SpawInfo);
		UE_LOG(Log_Spawner, Error, TEXT("%s 몬스터 스폰 A"), *FString(__FUNCTION__));
	}

	UE_LOG(Log_Spawner, Error, TEXT("%s 몬스터 스폰 B"), *FString(__FUNCTION__));
}

void AMonsterSpawner::OnSpawned(AActor* SpawnedActor)
{
	if (IsValid(SpawnedActor) == false)
	{
		return;
	}

	SpawnedActor->OnEndPlay.AddDynamic(this, &AMonsterSpawner::RemoveSpawnedActor);
}

void AMonsterSpawner::RemoveSpawnedActor(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	SpawnedActors.Remove(Actor);

	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
	check(GameState);

	if (URoundComponent* RoundComponent = GameState->GetComponentByClass<URoundComponent>())
	{
		RoundComponent->TryNextRound(this);
	}
}

bool AMonsterSpawner::IsClear_Implementation()
{
	return SpawnedActors.Num() == 0;
}

FTransform AMonsterSpawner::GetSpawnTransform()
{
	FTransform Transform = Super::GetSpawnTransform();

	if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this))
	{
		FNavLocation NavLocation;
		if (NavigationSystem->GetRandomReachablePointInRadius(GetActorLocation(), 3000.f, NavLocation))
		{
			Transform.SetLocation(NavLocation.Location);
		}
	}

	// 회전
	if (AActor* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		Transform.SetRotation(UKismetMathLibrary::FindLookAtRotation(Transform.GetLocation(), PlayerCharacter->GetActorLocation()).Quaternion());
	}

	return Transform;
}
