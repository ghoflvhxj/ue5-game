// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "Drop.generated.h"

USTRUCT(BlueprintType)
struct TESTGAME_API FDropTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int32, float> Porbs;

public:
	static const FDropTableRow Empty;
};

UINTERFACE(BlueprintType)
class TESTGAME_API UDropInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IDropInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	int32 GetDropIndex();
};