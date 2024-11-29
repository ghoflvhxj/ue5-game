// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "ActionComponent.generated.h"

USTRUCT(BlueprintType)
struct TESTGAME_API FMActionInfo // GameplayTag -> Montage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ActionTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimationAsset> ActionMontage;
};

UCLASS()
class TESTGAME_API UMActionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FMActionInfo> ActionInfos;
};

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class TESTGAME_API UMActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintPure)
	UAnimMontage* GetActionMontage(FGameplayTag ActionGameplayTag);
	UFUNCTION(BlueprintPure)
	UAnimSequence* GetActionSequence(FGameplayTag ActionGameplayTag);
	UFUNCTION(BlueprintPure)
	UObject* GetAction(FGameplayTag ActionGameplayTag);
protected:
	template <class T>
	T* GetAnimAsset(FGameplayTag ActionGameplayTag)
	{
		return ActionMap.Contains(ActionGameplayTag) ? Cast<T>(ActionMap[ActionGameplayTag].Get()) : nullptr;
	}

public:
	void UpdateAction(UMActionComponent* InActionComponent);
	void UpdateActionData(UMActionDataAsset* IntActionDataAsset);
private:
	void AddActions(const TArray<FMActionInfo>& InActionInfos);

protected:
	UPROPERTY(EditDefaultsOnly)
	UMActionDataAsset* ActionData;
	TMap<FGameplayTag, TObjectPtr<UAnimationAsset>> ActionMap;
};
