// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"

#include "ItemBase.generated.h"

USTRUCT(Atomic, BlueprintType)
struct FItemBaseInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemIndex = -1;
};

UCLASS()
class CLIENT_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AItemBase();

// 아이템 정보
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemBaseInfo ItemBaseInfo;
};

