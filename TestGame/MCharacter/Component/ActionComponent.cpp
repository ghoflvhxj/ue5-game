#include "ActionComponent.h"

void UMActionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ActionData))
	{
		AddActions(ActionData->ActionInfos);
	}
}

UAnimMontage* UMActionComponent::GetActionMontage(FGameplayTag ActionGameplayTag) const
{
	return GetAnimAsset<UAnimMontage>(ActionGameplayTag);
}

UAnimSequence* UMActionComponent::GetActionSequence(FGameplayTag ActionGameplayTag) const
{
	return GetAnimAsset<UAnimSequence>(ActionGameplayTag);
}

UObject* UMActionComponent::GetAction(FGameplayTag ActionGameplayTag) const
{
	return GetAnimAsset<UObject>(ActionGameplayTag);
}

void UMActionComponent::UpdateAction(UMActionComponent* InActionComponent)
{
	if (IsValid(InActionComponent) && IsValid(InActionComponent->ActionData))
	{
		AddActions(InActionComponent->ActionData->ActionInfos);
	}
}

void UMActionComponent::UpdateActionData(UMActionDataAsset* IntActionDataAsset)
{
	ActionData = IntActionDataAsset;

	if (HasBegunPlay())
	{
		ActionMap.Empty();

		if (IsValid(ActionData))
		{
			AddActions(ActionData->ActionInfos);
		}
	}
}

void UMActionComponent::AddActions(const TArray<FMActionInfo>& InActionInfos)
{
	for (const FMActionInfo& ActionInfo : InActionInfos)
	{
		ActionMap.FindOrAdd(ActionInfo.ActionTag) = ActionInfo.ActionMontage;
	}
}
