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
	using Super::MakeEffectContext;
	FGameplayEffectContextHandle MakeEffectContext(int32 InEffectIndex) const;

	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
};