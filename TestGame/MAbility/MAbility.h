// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TestGame/TestGame.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActivate;

	// server, client, all
};

UCLASS()
class TESTGAME_API UMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMAbilityBindInfo> Abilities;

	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const
	{
		checkf(IsValid(AbilitySystemComponent), TEXT("ASC Invalid."));
		AActor* Actor = AbilitySystemComponent->GetOwner<AActor>();
		checkf(IsValid(Actor), TEXT("ASC Invalid."));

		for (const FMAbilityBindInfo& BindInfo : Abilities)
		{
			if (IsValid(BindInfo.GameplayAbilityClass) == false)
			{
				UE_LOG(LogGAS, Warning, TEXT("Trying give Invalid AbilityClass to %s"), *(Actor->GetActorNameOrLabel()));
				continue;
			}

			FGameplayTag GameplayTag = BindInfo.GameplayTag;

			Handles.Emplace(GameplayTag, AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass)));
			if (BindInfo.bActivate && Handles.Contains(GameplayTag))
			{
				AbilitySystemComponent->TryActivateAbility(Handles[GameplayTag], true);
			}
		}
	}
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

public:
	UGameplayAbility_MoveToMouse();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void MoveToMouse(FGameplayEventData Payload);
};

UCLASS()
class TESTGAME_API UGameplayAbility_CharacterAction : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CharacterAction();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	virtual void Action(FGameplayEventData Payload);

protected:
	class UAbilityTask_WaitGameplayEvent* WaitTask;
	class UAbilityTask_PlayMontageAndWait* PlayMontageTask;
};

UCLASS()
class TESTGAME_API UGameplayAbility_BasicAttack : public UGameplayAbility_CharacterAction
{
	GENERATED_BODY()

public:
	virtual void Action(FGameplayEventData Payload) override;

private:
	void StartBasicAttack(const FGameplayEventData& Payload);
	void CancelBasicAttack();
	void EndBasicAttack();
};