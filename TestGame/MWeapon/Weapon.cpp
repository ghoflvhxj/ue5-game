// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon.h"
//#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"

DECLARE_LOG_CATEGORY_CLASS(Log_Weapon, Log, Log);

AWeapon::AWeapon()
{
	ActionComponent = CreateDefaultSubobject<UMActionComponent>(TEXT("ActionComponent"));
}

bool AWeapon::Attack()
{
	if (IsAttackable() == false || IsValid(GetOwner()) == false)
	{
		return false;
	}

	//if (UMActionComponent* ActionComponent = GetOwner()->GetComponentByClass<UMActionComponent>())
	//{
	//	UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0));
	//}

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AWeapon::OnAttackCoolDownFinished, AttackSpeed, false);

	return true;
}

void AWeapon::OnAttackCoolDownFinished()
{
	bAttackable = true;
}

bool AWeapon::IsAttackable() const
{
	return bAttackable;
}
