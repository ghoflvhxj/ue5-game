#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "MAbilitySystemGlobals.generated.h"

UCLASS()
class TESTGAME_API UMAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};