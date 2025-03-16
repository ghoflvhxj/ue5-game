#include "MAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

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

void UMAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters, EGameplayCueExecutionOptions::IgnoreSuppression);
}
