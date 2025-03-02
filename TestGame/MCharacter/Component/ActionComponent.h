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
	UFUNCTION(BlueprintPure, meta=(BLueprintThreadSafe))
	UAnimMontage* GetActionMontage(FGameplayTag ActionGameplayTag) const;
	UFUNCTION(BlueprintPure)
	UAnimSequence* GetActionSequence(FGameplayTag ActionGameplayTag) const;
	UFUNCTION(BlueprintPure)
	UObject* GetAction(FGameplayTag ActionGameplayTag) const;
protected:
	template <class T>
	T* GetAnimAsset(FGameplayTag ActionGameplayTag) const
	{
		return ActionMap.Contains(ActionGameplayTag) ? Cast<T>(ActionMap[ActionGameplayTag]) : nullptr;
	}

public:
	void CopyActions(TMap<FGameplayTag, UAnimationAsset*>& InActionMap) { InActionMap = ActionMap; }
	void UpdateAction(UMActionComponent* InActionComponent);
	void UpdateActionData(UMActionDataAsset* IntActionDataAsset);
private:
	void AddActions(const TArray<FMActionInfo>& InActionInfos);

protected:
	UPROPERTY(EditDefaultsOnly)
	UMActionDataAsset* ActionData;
	TMap<FGameplayTag, UAnimationAsset*> ActionMap;
};
