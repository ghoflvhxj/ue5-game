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

	for (int32 SpawnCounter = 0; SpawnCounter < InSpawnInfo.SpawnNum; ++SpawnCounter)
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

FTransform ASpawner::GetSpawnTransform()
{
	FTransform Transform = GetActorTransform();
	Transform.SetScale3D(FVector::OneVector);

	return Transform;
}

void ARoundSpanwer::BeginPlay()
{
	Super::BeginPlay();

	if (bActivated == false)
	{
		return;
	}

	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
	check(GameState);

	if (HasAuthority())
	{
		if (URoundComponent* RoundComponent = GameState->GetComponentByClass<URoundComponent>())
		{
			RoundComponent->OnRoundAndWaveChangedEvent.AddUObject(this, &ARoundSpanwer::SpawnUsingRoundInfo);
		}
	}
}

void AMonsterSpawner::SpawnUsingRoundInfo(const FRound& InRound)
{
	if (IsValid(MonsterSpawnTable) == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("%s 몬스터 스폰 테이블이 유효하지 않음"), *FString(__FUNCTION__));
		return;
	}

	FGameplayTag BossTag = FGameplayTag::RequestGameplayTag("Monster.Grade.Boss");
	if (FSpawnInfos* SpawnInfos = MonsterSpawnTable->FindRow<FSpawnInfos>(FName(FString::Printf(TEXT("%d-%d"), InRound.Round, InRound.Wave)), TEXT("MonsterSpawnTable")))
	{
		for (const FSpawnInfo& SpawnInfo : SpawnInfos->SpawnInfos)
		{
			Spawn(SpawnInfo);

			if (SpawnInfo.GradeTag == BossTag)
			{
				BossMonster = SpawnedActors.Last().Get();
				OnBossSpawnedEvent.Broadcast(BossMonster);
			}
		}
	}

	LastSpawnWave = InRound.Wave;
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

	if (URoundComponent* RoundComponent = GetRoundComponent())
	{
		if (RoundComponent->IsLastWaveFor(LastSpawnWave) && SpawnedActors.IsEmpty())
		{
			RoundComponent->TryNextRound(10.f);
		}
	}
}

URoundComponent* AMonsterSpawner::GetRoundComponent()
{
	if (AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		return GameState->GetComponentByClass<URoundComponent>();
	}

	return nullptr;
}

//bool AMonsterSpawner::IsClear_Implementation()
//{
//	return SpawnedActors.Num() == 0;
//}

FTransform AMonsterSpawner::GetSpawnTransform()
{
	FTransform Transform = Super::GetSpawnTransform();

	//if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this))
	//{
	//	for (int i = 0; i < 5; ++i)
	//	{
	//		FNavLocation NavLocation;
	//		FVector Origin = GetActorLocation();

	//		float Distance = FMath::FRandRange(MinRadius, MaxRadius);
	//		float Angle = FMath::FRand() * 90.f;

	//		if (NavigationSystem->GetRandomReachablePointInRadius(Origin, MaxRadius, NavLocation))
	//		{
	//			if (MinRadius <= FVector::Distance(Origin, NavLocation.Location) && FVector::Distance(Origin, NavLocation.Location) <= MaxRadius)
	//			{
	//				Transform.SetLocation(NavLocation.Location);
	//				break;
	//			}
	//		}
	//	}
	//}

	if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this))
	{
		if (ANavigationData* NavData = NavigationSystem->GetDefaultNavDataInstance())
		{
			FNavLocation NavLocation;
			FVector Origin = GetActorLocation();

			float Distance = FMath::FRandRange(MinRadius, MaxRadius);
			float Angle = FMath::FRand() * 360.f;

			int Interval = 90;
			int LoopCount = 360 / Interval;
			for (int i = 0; i < LoopCount; ++i)
			{
				FPathFindingQuery Query;
				Query.StartLocation = GetActorLocation();
				Query.EndLocation = GetActorLocation() + FRotator(0.0, Angle + (i * Interval), 0.0).RotateVector(FVector::XAxisVector) * Distance; 
				Query.QueryFilter = NavData->GetDefaultQueryFilter();
				FPathFindingResult PathFindingResult = NavigationSystem->FindPathSync(Query);
				if (PathFindingResult.IsSuccessful())
				{
					Transform.SetLocation(PathFindingResult.Path->GetPathPoints().Last().Location);
					break;
				}
			}
		}
	}

	// 회전
	if (AActor* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		Transform.SetRotation(UKismetMathLibrary::FindLookAtRotation(Transform.GetLocation(), PlayerCharacter->GetActorLocation()).Quaternion());
	}

	return Transform;
}

#if WITH_EDITOR

void AMonsterSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FlushPersistentDebugLines(GetWorld());
	for (int i=0; i< SpawnPositionNum; ++i)
	{
		DrawDebugSphere(GetWorld(), GetSpawnTransform().GetLocation(), 100.f, 4, FColor::Blue, true);
	}
}

void AMonsterSpawner::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished)
	{
		FlushPersistentDebugLines(GetWorld());
		for (int i = 0; i < SpawnPositionNum; ++i)
		{
			DrawDebugSphere(GetWorld(), GetSpawnTransform().GetLocation(), 100.f, 4, FColor::Blue, true);
		}
	}
}

#endif