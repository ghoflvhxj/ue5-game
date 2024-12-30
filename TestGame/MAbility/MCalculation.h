#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MCalculation.generated.h"

UCLASS()
class TESTGAME_API UMGameplayEffectExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};