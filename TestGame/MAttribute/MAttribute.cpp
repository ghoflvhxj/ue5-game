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

	if (AActor* Owner = GetOwningActor())
	{
		UE_CLOG(Owner, LogAttribute, Warning, TEXT("%s, AttributeChanged:%s %f->%f"), *Owner->GetName(), *Attribute.GetName(), OldValue, NewValue);
	}
}

void UMAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMAttributeSet, MaxHealth);
	DOREPLIFETIME(UMAttributeSet, Health);
	DOREPLIFETIME(UMAttributeSet, AttackPower);
	DOREPLIFETIME(UMAttributeSet, SkillPower);
	DOREPLIFETIME(UMAttributeSet, MoveSpeed);
	DOREPLIFETIME(UMAttributeSet, AttackMoveSpeed);
	DOREPLIFETIME(UMAttributeSet, BasicAttackSpeed);
}

void UMAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, Health, OldValue);
}

void UMAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, MoveSpeed, OldValue);
}

void UMWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMWeaponAttributeSet, Ammo);
}