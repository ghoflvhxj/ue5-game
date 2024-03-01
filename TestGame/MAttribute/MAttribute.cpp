// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAttribute.h"

void UMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (APawn* Character = Cast<APawn>(GetOwningActor()))
	{
		if (const UAbilitySystemComponent* AbilitySystemComponent = Character->FindComponentByClass<UAbilitySystemComponent>())
		{
			if (AbilitySystemComponent->DefaultStartingData.IsEmpty())
			{

			}

			// 첫번쨰 꺼만 찾는 거는 약간 이상한듯
			if (AbilitySystemComponent->DefaultStartingData[0].DefaultStartingTable != nullptr && AbilitySystemComponent->DefaultStartingData[0].Attributes != nullptr)
			{
				FString RowName = AbilitySystemComponent->DefaultStartingData[0].Attributes->GetName() + TEXT(".") + Attribute.GetName();
				FAttributeMetaData* AttributeMetaData = AbilitySystemComponent->DefaultStartingData[0].DefaultStartingTable->FindRow<FAttributeMetaData>(FName(RowName), TEXT(""));

				if (AttributeMetaData != nullptr)
				{
					NewValue = FMath::Clamp(NewValue, AttributeMetaData->MinValue, AttributeMetaData->MaxValue);
				}
			}
		}
	}
}

//void UMAttributeSet::AddAttributeChangedDelegate(UObject* Object, FOnAttributeChangedDelegate Functor)
//{
//	OnAttributeChangedDelegate.AddDynamic(Object, Functor);
//}
