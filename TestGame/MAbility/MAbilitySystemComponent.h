#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MEffectTypes.h"
#include "MAbilitySystemComponent.generated.h"

UCLASS()
class TESTGAME_API UMAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContextHandle MakeEffectContext() const override;
	FGameplayEffectContextHandle MakeEffectContext(int32 InEffectIndex) const;
};