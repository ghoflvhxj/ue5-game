#include "MAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

FGameplayEffectContextHandle UMAbilitySystemComponent::MakeEffectContext() const
{
	FMGameplayEffectContextHandle Context = FMGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());

	// By default use the owner and avatar as the instigator and causer
	if (ensureMsgf(AbilityActorInfo.IsValid(), TEXT("Unable to make effect context because AbilityActorInfo is not valid.")))
	{
		Context.AddInstigator(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get());
	}

	return Context;
}

FGameplayEffectContextHandle UMAbilitySystemComponent::MakeEffectContext(int32 InEffectIndex) const
{
	FMGameplayEffectContextHandle Context = FMGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());

	if (ensureMsgf(AbilityActorInfo.IsValid(), TEXT("Unable to make effect context because AbilityActorInfo is not valid.")))
	{
		Context.AddInstigator(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get());
	}

	Context.SetEffectIndex(InEffectIndex);

	return Context;
}
