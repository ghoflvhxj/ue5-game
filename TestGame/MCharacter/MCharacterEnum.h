// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "MCharacterEnum.generated.h"

UENUM(BlueprintType)
enum class ECharacterVitalityState : uint8
{
	Alive,
	Die,
	Count
};

//UENUM(BlueprintType)
//enum class ECharacterPoseState : uint8
//{
//	Stand,
//	Crouch,
//	Lie,
//	Airborne,
//	Count
//};

//UENUM(BlueprintType)
//enum class ECharacterMovementState : uint8
//{
//	Idle,
//	Move,
//	FastMove,
//	Fall,
//	Up,
//	Count
//};

//UENUM(BlueprintType)
//enum class ECharacterActionState : uint8
//{
//	Attack,
//	Skill,
//	Attacked,
//	Count
//};