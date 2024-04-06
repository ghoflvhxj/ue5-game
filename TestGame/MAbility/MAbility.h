// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "Engine/Classes/Engine/DataAsset.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MAbility.generated.h"

class UGameplayAbility;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct TESTGAME_API FMAbilityBindInfo
{
	GENERATED_BODY();

public:
	FMAbilityBindInfo()
		: GameplayTag()
	{

	}

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GameplayTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;
};

UCLASS()
class TESTGAME_API UMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMAbilityBindInfo> Abilities;

	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAbilitySpecHandle>& Handles) const
	{
		checkf(IsValid(AbilitySystemComponent), TEXT("ASC is null"));

		for (const FMAbilityBindInfo& BindInfo : Abilities)
		{
			if (BindInfo.GameplayAbilityClass)
			{
				Handles.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass)));
			}
		}
	}

	//TSubclassOf<UGameplayAbility> GetAbilityClassBy
};

class AMCharacter;

USTRUCT(BlueprintType)
struct TESTGAME_API FTableRow_MCharacterToAbilitySet : public FTableRowBase
{
	GENERATED_BODY()

	FTableRow_MCharacterToAbilitySet()
		: FTableRowBase()
	{}


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AMCharacter>, UMAbilityDataAsset*> MCharacterClassToAbilityDataAsset;
};

UCLASS()
class TESTGAME_API UGameplayAbility_MoveToMouse : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void MoveToMouse(FGameplayEventData Payload);
};