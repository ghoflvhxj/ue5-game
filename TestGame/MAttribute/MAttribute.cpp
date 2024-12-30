#pragma once

#include "MAttribute.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogAttribute);

void UMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UMAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (AActor* Owner = GetOwningActor())
	{
		UE_CLOG(Owner, LogAttribute, VeryVerbose, TEXT("%s, AttributeChanged:%s %f->%f"), *Owner->GetName(), *Attribute.GetName(), OldValue, NewValue);
	}
}

void UMAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMAttributeSet, MaxHealth);
	DOREPLIFETIME(UMAttributeSet, Health);
	DOREPLIFETIME(UMAttributeSet, AttackPower);
	DOREPLIFETIME(UMAttributeSet, MoveSpeed);
	DOREPLIFETIME(UMAttributeSet, AttackSpeed);
}

void UMAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, MaxHealth, OldData);
}

void UMAttributeSet::OnRep_Health(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, Health, OldData);
}

void UMAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, MoveSpeed, OldData);
}

void UMAttributeSet::OnRep_WeaponScale(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMAttributeSet, WeaponScale, OldData);
}

void UMWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMWeaponAttributeSet, Ammo);
}