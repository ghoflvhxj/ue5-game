// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "MGameInstance.generated.h"

struct FItemBaseInfo;

UCLASS(Blueprintable)
class TESTGAME_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

public:
	FItemBaseInfo* GetItemBaseInfo(FName InRowName);
	FItemBaseInfo* GetItemBaseInfo(int32 InItemIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable = nullptr;
	TMap<int32, FName> ItemIndexToNameMap;
};
