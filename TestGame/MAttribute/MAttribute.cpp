// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

DEFINE_LOG_CATEGORY(LogAttribute);

#include "MAttribute.h"
#include "Net/UnrealNetwork.h"

void UMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UMAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UE_LOG(LogAttribute, Warning, TEXT("%s, AttributeChanged:%s %f->%f"), GetOwningActor() ? *GetOwningActor()->GetName() : *FString(TEXT("None")), *Attribute.GetName(), OldValue, NewValue);
}

void UMAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMAttributeSet, MaxHealth);
	DOREPLIFETIME(UMAttributeSet, Health);
	DOREPLIFETIME(UMAttributeSet, AttackPower);
	DOREPLIFETIME(UMAttributeSet, SkillPower);
	DOREPLIFETIME(UMAttributeSet, MoveSpeed);
	DOREPLIFETIME(UMAttributeSet, BasicAttackSpeed);
}

//void UMAttributeSet::AddAttributeChangedDelegate(UObject* Object, FOnAttributeChangedDelegate Functor)
//{
//	OnAttributeChangedDelegate.AddDynamic(Object, Functor);
//}
