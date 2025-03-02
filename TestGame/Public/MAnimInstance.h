// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "MAnimInstance.generated.h"

class UMActionDataAsset;

UCLASS()
class TESTGAME_API UMAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;

public:
	UFUNCTION()
	void AnimNotify_Combo();
	UFUNCTION()
	void AnimNotify_Ragdoll();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, UAnimationAsset*> Animations;

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMActionDataAsset* Actions = nullptr;
#endif
};

//UCLASS()
//class TESTGAME_API UMonsterAnimInstnace : public UMAnimInstance
//{
//	GENERATED_BODY()
//
//public:
//	virtual void NativeInitializeAnimation() override;
//
//#if WITH_EDITORONLY_DATA
//protected:
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	UDataTable* MonsterTable = nullptr;
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	int32 MonsterIndex = INDEX_NONE;
//#endif
//};