// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAttribute.h"

void UMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UMAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UE_LOG(LogTemp, Warning, TEXT("%s %f->%f"), *Attribute.GetName(), OldValue, NewValue);
}

//void UMAttributeSet::AddAttributeChangedDelegate(UObject* Object, FOnAttributeChangedDelegate Functor)
//{
//	OnAttributeChangedDelegate.AddDynamic(Object, Functor);
//}
