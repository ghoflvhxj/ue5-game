#include "ActionComponent.h"

void UMActionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ActionData))
	{
		for (const FMActionInfo& ActionInfo : ActionData->ActionInfos)
		{
			ActionMap.FindOrAdd(ActionInfo.ActionTag) = Cast<UAnimMontage>(ActionInfo.ActionMontage);
		}
	}
}

UAnimMontage* UMActionComponent::GetActionMontage(FGameplayTag ActionGameplayTag)
{
	return ActionMap.Contains(ActionGameplayTag) ? ActionMap[ActionGameplayTag] : nullptr;
}
