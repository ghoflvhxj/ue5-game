// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterSpawner.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "Async/Async.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

#include "MGameInstance.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MMonster.h"
#include "TestGame/MGameState/MGameStateInGame.h"

DECLARE_LOG_CATEGORY_CLASS(Log_Spawner, Log, Log);

ASpawner::ASpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() == false)
	{
		SetActorTickEnabled(false);
		return;
	}

	if (GetWorld()->GetTimeSeconds() - LastSpawnTime < 0.01f)
	{
		return;
	}

	for (int i = 2; i > 0; --i)
	{
		if (SpawnPool.IsEmpty() == false)
		{
			const FSpawnInfo& SpawnInfo = SpawnPool.Pop();
			if (AActor* SpawnedActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(this, SpawnInfo.SpawnClass, SpawnInfo.Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
			{
				SpawnedActors.Add(SpawnedActor);
				OnSpawnedActorConstruction(SpawnedActor);
				UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnInfo.Transform);
				OnSpawned(SpawnedActor);
			}
		}
	}

	LastSpawnTime = GetWorld()->GetTimeSeconds();
}

void ASpawner::AddSpawnInfo(const FSpawnInfo& InSpawnInfo)
{
	if (bActivated == false)
	{
		return;
	}

	SpawnPool.Add(InSpawnInfo);

	SetActorTickEnabled(true);
}

void ASpawner::AddSpawnInfo(const TArray<FSpawnInfo>& InSpawnInfos)
{
	if (bActivated == false)
	{
		return;
	}

	SpawnPool.Append(InSpawnInfos);
}

FTransform ASpawner::GetSpawnTransform()
{
	FTransform Transform = GetActorTransform();
	Transform.SetScale3D(FVector::OneVector);

	return Transform;
}

const TSet<AActor*>& ASpawner::GetSpawnedActors()
{
	return SpawnedActors;
}

void ARoundSpanwer::BeginPlay()
{
	Super::BeginPlay();

	if (bActivated == false)
	{
		return;
	}

	if (URoundComponent* RoundComponent = GetRoundComponent())
	{
		RoundComponent->OnRoundAndWaveChangedEvent.AddUObject(this, &ARoundSpanwer::SpawnUsingRoundInfo);
	}
}

void ARoundSpanwer::SpawnUsingRoundInfo(const FRound& InRound)
{
	CachedRound = InRound;
}

URoundComponent* ARoundSpanwer::GetRoundComponent()
{
	if (AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		return GameState->GetComponentByClass<URoundComponent>();
	}

	return nullptr;
}

void AMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (URoundComponent* RoundComponent = GetRoundComponent())
	{
		RoundComponent->GetWaitNextRoundEvent().AddWeakLambda(this, [this]() {
			LoadRoundMonsters(CachedRound.Round + 1);
		});
	}
}

void AMonsterSpawner::SpawnUsingRoundInfo(const FRound& InRound)
{
	if (HasAuthority() == false)
	{
		return;
	}

	Super::SpawnUsingRoundInfo(InRound);

	UMGameInstance* GameInstance = Cast<UMGameInstance>(GetGameInstance());

	if (IsValid(GameInstance) == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("%s invalid game instance."), *FString(__FUNCTION__));
		return;
	}

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this);
	if (IsValid(NavigationSystem) == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("%s invalid Nav."), *FString(__FUNCTION__));
		return;
	}

	if (FRoundMonsterSpawnTableRow * RoundMonsterSpawnTableRow = GetCurrentRoundMonsterSpawn())
	{
		for (const TPair<int32, int32> IndexToNumPair : RoundMonsterSpawnTableRow->MonsterIndexToSpawnNum)
		{
			FSpawnInfo SpawnInfo;

			int32 Monsterindex = IndexToNumPair.Key;
			int32 SpawnNum = IndexToNumPair.Value;

			FVector Offset = FVector::ZeroVector;
			const FMonsterTableRow& MonsterTableRow = GameInstance->GetMonsterTableRow(Monsterindex);
			if (UClass* MonsterClass = MonsterTableRow.MonsterData.MonsterClass.TryLoadClass<AMMonster>())
			{
				SpawnInfo.SpawnClass = MonsterClass;
				if (ACharacter* Character = MonsterClass->GetDefaultObject<ACharacter>())
				{
					if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
					{
						Offset.Z = CapsuleComponent->GetScaledCapsuleHalfHeight();
					}
				}
			}

			SpawnInfo.SpawnTags = MonsterTableRow.Tags;

			TArray<FSpawnInfo> SpawnInfos;
			SpawnInfos.Reserve(SpawnNum);
			if (ANavigationData* NavData = NavigationSystem->GetDefaultNavDataInstance())
			{
				FVector Origin = GetActorLocation();

				FNavLocation NavigableLocation;
				if (NavigationSystem->ProjectPointToNavigation(Origin, NavigableLocation) == false)
				{
					if (NavigationSystem->GetRandomPointInNavigableRadius(Origin, 500.f, NavigableLocation, NavData))
					{
						Origin = NavigableLocation.Location;
					}
				}

				for (int32 Counter = 0; Counter < SpawnNum; ++Counter)
				{
					FNavLocation NavLocation;

					float Distance = FMath::FRandRange(MinRadius, MaxRadius);
					float Angle = FMath::FRand() * 360.f;

					int Interval = 90;
					int LoopCount = 360 / Interval;
					for (int i = 0; i < LoopCount; ++i)
					{
						FPathFindingQuery Query;
						Query.StartLocation = Origin;
						Query.EndLocation = Origin + FRotator(0.0, Angle + (i * Interval), 0.0).RotateVector(FVector::XAxisVector) * Distance;
						Query.QueryFilter = NavData->GetDefaultQueryFilter();
						Query.NavData = NavData;
						FPathFindingResult PathFindingResult = NavigationSystem->FindPathSync(Query);
						if (PathFindingResult.IsSuccessful())
						{
							if (PathFindingResult.Path->GetLength() > MaxRadius)
							{
								continue;
							}
							SpawnInfo.Transform.SetLocation(PathFindingResult.Path->GetPathPoints().Last().Location + Offset);
							break;
						}
					}

					SpawnInfos.Add(SpawnInfo);
					MonsterIndexPool.Add(Monsterindex);
				}
			}

			AddSpawnInfo(SpawnInfos);
		}
	}

	LastSpawnWave = InRound.Wave;
}

void AMonsterSpawner::OnSpawnedActorConstruction(AActor* SpawnedActor)
{
	if (MonsterIndexPool.IsEmpty() == false)
	{
		if (AMMonster* Monster = Cast<AMMonster>(SpawnedActor))
		{
			Monster->SetMonsterIndex(MonsterIndexPool.Pop());
		}
	}
}

void AMonsterSpawner::OnSpawned(AActor* SpawnedActor)
{
	if (AMMonster* Monster = Cast<AMMonster>(SpawnedActor))
	{
		Monster->PlayStartAnim();
		Monster->OnEndPlay.AddDynamic(this, &AMonsterSpawner::RemoveSpawnedActor);
	}
}

void AMonsterSpawner::RemoveSpawnedActor(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	SpawnedActors.Remove(Actor);

	if (URoundComponent* RoundComponent = GetRoundComponent())
	{
		if (RoundComponent->IsLastWaveFor(LastSpawnWave) && SpawnedActors.IsEmpty())
		{
			RoundComponent->FinishRound();
		}
	}
}

FRoundMonsterSpawnTableRow* AMonsterSpawner::GetRoundMonsterSpawn(const FRound& InRound)
{
	if (IsValid(MonsterSpawnTable))
	{
		return MonsterSpawnTable->FindRow<FRoundMonsterSpawnTableRow>(*FString::Printf(TEXT("%d-%d"), InRound.Round, InRound.Wave), TEXT("RoundMonsterSpawnTable"));;
	}

	return nullptr;
}

FRoundMonsterSpawnTableRow* AMonsterSpawner::GetCurrentRoundMonsterSpawn(int32 RoundOffest)
{
	return GetRoundMonsterSpawn(CachedRound);
}

void AMonsterSpawner::LoadRoundMonsters(int32 InRound)
{
	TArray<int32> MonsterIndices;

	for (int32 i = 1; i < 10; ++i)
	{
		FRound Round;
		Round.Round = InRound;
		Round.Wave = i;

		if (FRoundMonsterSpawnTableRow* MonsterSpawnTableRow = GetRoundMonsterSpawn(Round))
		{
			TArray<int32> TempIndices;
			MonsterSpawnTableRow->MonsterIndexToSpawnNum.GenerateKeyArray(TempIndices);
			MonsterIndices.Append(TempIndices);
		}
		else
		{
			break;
		}
	}

	TSet<FSoftClassPath> MonsterClasses;
	if (UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>())
	{
		for (int32 MonsterIndex : MonsterIndices)
		{
			const FMonsterTableRow MonsterTableRow = GameInstance->GetMonsterTableRow(MonsterIndex);
			MonsterClasses.Add(MonsterTableRow.MonsterData.MonsterClass);
		}
	}
	
	for (FSoftClassPath MonsterClassPath : MonsterClasses)
	{
		AsyncTask(ENamedThreads::GameThread, [MonsterClassPath, this]() {
			MonsterClassPath.TryLoadClass<AMMonster>();
		});
		UE_LOG(LogTemp, VeryVerbose, TEXT("%s : %s"), *FString(__FUNCTION__), *MonsterClassPath.GetAssetName())
	}

	//FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	//if (UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>())
	//{
	//	for (int32 MonsterIndex : MonsterIndices)
	//	{
	//		const FMonsterTableRow MonsterTableRow = GameInstance->GetMonsterTableRow(MonsterIndex);
	//		//UKismetSystemLibrary::LoadAssetClass(this, MonsterTableRow.MonsterData.MonsterClass.TryLoad());
	//		FSoftObjectPath ObjectPath = MonsterTableRow.MonsterData.MonsterClass.ToString();

	//			Streamable.RequestAsyncLoad(ObjectPath, [MonsterTableRow]() {

	//		});
	//	}
	//}
}

bool AMonsterSpawner::IsBossContain(const UObject* WorldContext, const FRoundMonsterSpawnTableRow& InSpawnInfos)
{
	UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));

	if (IsValid(GameInstance) == false)
	{
		UE_LOG(Log_Spawner, Error, TEXT("%s using invalid game instance"), *FString(__FUNCTION__));
		return false;
	}

	FGameplayTag BossTag = FGameplayTag::RequestGameplayTag("Monster.Grade.Boss");

	for (const TPair<int32, int32> IndexToNumPair : InSpawnInfos.MonsterIndexToSpawnNum)
	{
		const FMonsterTableRow& MonsterTableRow = GameInstance->GetMonsterTableRow(IndexToNumPair.Key);
		if (MonsterTableRow.Tags.HasTag(BossTag))
		{
			return true;
		}
	}

	return false;
}

FTransform AMonsterSpawner::GetSpawnTransform()
{
	FTransform Transform = Super::GetSpawnTransform();



	// 회전, 가까이 있는 플레이어 캐릭터를 찾아야 함
	//if (AActor* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	//{
	//	Transform.SetRotation(UKismetMathLibrary::FindLookAtRotation(Transform.GetLocation(), PlayerCharacter->GetActorLocation()).Quaternion());
	//}

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