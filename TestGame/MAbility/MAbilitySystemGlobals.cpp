#include "MAbilitySystemGlobals.h"
#include "MEffectTypes.h"

FGameplayEffectContext* UMAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMGameplayEffectContext();
}
