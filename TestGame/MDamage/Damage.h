// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "Damage.generated.h"

UENUM(BlueprintType)
enum class EThrustCoord : uint8
{
	SelfXZ,
	SelfX,
	SelfZ,
	TargetXZ,
	TargetX,
	TargetZ
};

USTRUCT(BlueprintType)
struct FDamageRecord : public FTableRowBase
{
	GENERATED_BODY()

	//UPROPERTY(BlueprintReadWrite)
	//float ThrustPower;

	//UPROPERTY(BlueprintReadWrite)
	//FVector ThrustFactor;

	//UPROPERTY(BlueprintReadWrite)
	//EThrustCoord ThrustCoord;
};