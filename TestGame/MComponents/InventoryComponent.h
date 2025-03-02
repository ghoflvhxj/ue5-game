// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "InventoryComponent.generated.h"

DECLARE_EVENT_OneParam(UMInventoryComponent, FOnMoneyChangedEvent, int32);
DECLARE_EVENT_TwoParams(UMInventoryComponent, FOnItemAddedEvent, int32, int32);


UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class TESTGAME_API UMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMInventoryComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool AddMoney(int32 InAdditiveMoney);
	UFUNCTION()
	void OnRep_Money();
	FOnMoneyChangedEvent OnMoneyChangedEvent;
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int32 Money = 0;

public:
	void AddItem(int32 InIndex, int32 InNum);
	FOnItemAddedEvent& GetItemAddedEvent() { return OnItemAddedEvent; }
	UFUNCTION()
	void OnRep_Items();

	UFUNCTION(BlueprintPure)
	const TMap<int32, int32>& GetItemToLevelMap() { return ItemMap; }
	UFUNCTION(BlueprintPure)
	int32 GetItemLevel(int32 InItemIndex) const { return ItemMap.Contains(InItemIndex) ? ItemMap[InItemIndex] : INDEX_NONE; }
protected:
	TMap<int32, int32> ItemMap;
	TMap<int32, int32> CachedItemMap;
	FOnItemAddedEvent OnItemAddedEvent;
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<uint8> SerializeItems;

	void SerializeMap(FArchive& Archive);
};
