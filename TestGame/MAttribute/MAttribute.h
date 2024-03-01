// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "Engine/Classes/Engine/DataAsset.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MAttribute.generated.h"

class UGameplayAbility;
struct FGameplayTag;

UCLASS(DefaultToInstanced, Blueprintable)
class UMAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData SkillPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MoveSpeed;

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, MaxHealth)
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, Health)
};