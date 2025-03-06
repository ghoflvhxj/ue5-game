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

public:
	UFUNCTION()
	void AnimNotify_FootStep();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, UAnimationAsset*> Animations;

#if WITH_EDITORONLY_DATA
protected:
	// 에디터 전용 애니메이션 셋팅
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMActionDataAsset* Actions = nullptr;
#endif
};