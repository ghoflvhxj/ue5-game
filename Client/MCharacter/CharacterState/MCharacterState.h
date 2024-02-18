// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "Client/MCharacter/Component/StateMachineComponent.h"
#include "Client/Mcharacter/MCharacterEnum.h"


#include "MCharacterState.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CLIENT_API UCharacterVitalityState : public UStateClass
{
public:
	GENERATED_BODY()

public:
	UCharacterVitalityState()
		: UStateClass(StaticEnum<ECharacterVitalityState>())
	{

	}
};
