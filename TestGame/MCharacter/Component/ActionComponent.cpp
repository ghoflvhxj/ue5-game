#include "ActionComponent.h"

void UMActionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ActionData))
	{
		AddActions(ActionData->ActionInfos);
	}
}

UAnimMontage* UMActionComponent::GetActionMontage(FGameplayTag ActionGameplayTag)
{
	return ActionMap.Contains(ActionGameplayTag) ? ActionMap[ActionGameplayTag] : nullptr;
}

void UMActionComponent::UpdateAction(UMActionComponent* InActionComponent)
{
	if (IsValid(InActionComponent) && IsNetSimulating() == false && IsValid(InActionComponent->ActionData))
	{
		Multicast_UpdateAction(InActionComponent->ActionData->ActionInfos);
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

void UMActionComponent::Multicast_UpdateAction_Implementation(const TArray<FMActionInfo>& InActionInfos)
{
	AddActions(InActionInfos);
}

void UMActionComponent::AddActions(const TArray<FMActionInfo>& InActionInfos)
{
	for (const FMActionInfo& ActionInfo : InActionInfos)
	{
		ActionMap.FindOrAdd(ActionInfo.ActionTag) = Cast<UAnimMontage>(ActionInfo.ActionMontage);
	}
}
