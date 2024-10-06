// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "InventoryComponent.generated.h"

DECLARE_EVENT_OneParam(UMInventoryComponent, FOnMoneyChangedEvent, int32);

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
};
