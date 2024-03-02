#include "StateMachineComponent.h"
#include "Net/UnrealNetwork.h"

UStateComponent::UStateComponent()
{

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

	States.Add(0);
	StateEnumClassToIndexMap.Add(EnumClass, States.Num());

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
	if (StateIndex == INDEX_NONE)
	{
		return false;
	}

	if (States[StateIndex] != NewStateValue)
	{
		uint8 OldStateValue = States[StateIndex];
		States[StateIndex] = NewStateValue;

		if (StateChangedDelegatesMap.Contains(StateEnumClass))
		{
			StateChangedDelegatesMap[StateEnumClass].Broadcast(OldStateValue, NewStateValue);
		}
	}

	return true;
}

bool UStateComponent::GetStateInternal(UEnum* EnumClass, uint8& OutStateValue)
{
	int StateIndex = GetStateIndex(EnumClass);
	if (StateIndex == INDEX_NONE)
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

	return StateEnumClassToIndexMap.FindRef(EnumClass) - 1;
}

void UStateComponent::AddOnStateChangeDelegate(TSubclassOf<UStateClass> StateClass, UObject* Object, const TFunction<void(uint8, uint8)> Func)
{
	AddOnStateChangeDelegateInternal(StateClass->GetDefaultObject<UStateClass>()->EnumClass, Object, Func);
}

void UStateComponent::AddOnStateChangeDelegateInternal(UEnum* StateEnumClass, UObject* Object, const TFunction<void(uint8, uint8)> Func)
{
	//FOnStateChangedDelegate Delegate = FOnStateChangedDelegate::AddUObject(Object, Func);
	StateChangedDelegatesMap.FindOrAdd(StateEnumClass).AddWeakLambda(Object, Func);
}
