#include "StateMachineComponent.h"
#include "Net/UnrealNetwork.h"

UStateComponent::UStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UStateComponent::AddStateClass(TSubclassOf<UStateClass> StateClass)
{
	return AddStateClassInternal(StateClass->GetDefaultObject<UStateClass>()->EnumClass);
}

void UStateComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStateComponent, States);
}

bool UStateComponent::AddStateClassInternal(UEnum* EnumClass)
{
	if (EnumClass == nullptr || StateEnumClassToIndexMap.Contains(EnumClass))
	{
		return false;
	}

	if (IsNetSimulating() == false)
	{
		States.Add(0);
	}

	StateEnumClassToIndexMap.Add(EnumClass, StateEnumClassToIndexMap.Num());

	return true;
}

bool UStateComponent::ChangeState(TSubclassOf<UStateClass> StateClass, uint8 NewStateValue)
{
	return ChangeStateInternal(StateClass->GetDefaultObject<UStateClass>()->EnumClass, NewStateValue);
}

bool UStateComponent::GetState(TSubclassOf<UStateClass> StateClass, uint8& OutStateValue)
{
	return GetStateInternal(StateClass->GetDefaultObject<UStateClass>()->EnumClass, OutStateValue);
}

bool UStateComponent::ChangeStateInternal(UEnum* StateEnumClass, uint8 NewStateValue)
{
	int StateIndex = GetStateIndex(StateEnumClass);
	if (StateIndex != INDEX_NONE && States[StateIndex] != NewStateValue)
	{
		uint8 OldStateValue = States[StateIndex];
		if (IsNetSimulating() == false)
		{
			TArray<uint8> OldStates = States;
			States[StateIndex] = NewStateValue;
			OnStateUpdated(OldStates);

			return true;
		}
	}

	return false;
}

bool UStateComponent::GetStateInternal(UEnum* EnumClass, uint8& OutStateValue)
{
	int StateIndex = GetStateIndex(EnumClass);
	if (States.IsValidIndex(StateIndex) == false)
	{
		return false;
	}

	OutStateValue = States[StateIndex];
	return true;
}

int UStateComponent::GetStateIndex(UEnum* EnumClass)
{
	if (EnumClass == nullptr)
	{
		return INDEX_NONE;
	}

	return StateEnumClassToIndexMap.FindRef(EnumClass);
}

void UStateComponent::AddOnStateChangeDelegate(TSubclassOf<UStateClass> StateClass, UObject* Object, const TFunction<void(uint8, uint8)> Func)
{
	AddOnStateChangeDelegateInternal(StateClass->GetDefaultObject<UStateClass>()->EnumClass, Object, Func);
}

void UStateComponent::AddOnStateChangeDelegateInternal(UEnum* StateEnumClass, UObject* Object, const TFunction<void(uint8, uint8)> Func)
{
	//FOnStateChangedDelegate Delegate = FOnStateChangedDelegate::AddUObject(Object, Func);
	StateChangedDelegatesMap.FindOrAdd(GetStateIndex(StateEnumClass)).AddWeakLambda(Object, Func);
}

void UStateComponent::OnStateUpdated(TArray<uint8>& OldStates)
{
	TArray<int32> StateClassIndexArray;
	StateEnumClassToIndexMap.GenerateValueArray(StateClassIndexArray);

	for(int StateIndex = 0; StateIndex < States.Num(); ++StateIndex)
	{
		if (StateChangedDelegatesMap.Contains(StateIndex) == false)
		{
			continue;
		}

		if (OldStates.IsValidIndex(StateIndex) == false || OldStates[StateIndex] != States[StateIndex])
		{
			StateChangedDelegatesMap[StateIndex].Broadcast(OldStates[StateIndex], States[StateIndex]);
		}
	}
}
