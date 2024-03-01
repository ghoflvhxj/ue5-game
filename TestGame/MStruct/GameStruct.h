// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameStruct.generated.h"

//USTRUCT(BlueprintType)
//struct 

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol,
	Rifle,
	Shotgun,
	Machinegun
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Attack,
	Move,
	Jump,
	Dash,
};