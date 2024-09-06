// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon.h"
//#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"

DECLARE_LOG_CATEGORY_CLASS(Log_Weapon, Log, Log);

AWeapon::AWeapon()
{
	ActionComponent = CreateDefaultSubobject<UMActionComponent>(TEXT("ActionComponent"));

	bReplicates = true;
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UE_LOG(Log_Weapon, Warning, TEXT("Weapon Construct Index:%d"), WeaponIndex);

	if (HasAuthority())
	{
		if (FWeaponData* WeaponData = GetWeaponData())
		{
			if (USkeletalMeshComponent* SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>())
			{
				SkeletalMeshComponent->SetSkeletalMesh(WeaponData->Mesh);
			}
		}
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, WeaponIndex, COND_InitialOnly);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (FWeaponData* WeaponData = GetWeaponData())
	{
		if (IsValid(ActionComponent))
		{
			ActionComponent->UpdateActionData(WeaponData->ActionData);
		}
	}
}

void AWeapon::OnRep_WeaponIndex()
{
	if (HasAuthority() == false)
	{
		if (FWeaponData* WeaponData = GetWeaponData())
		{
			if (USkeletalMeshComponent* SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>())
			{
				SkeletalMeshComponent->SetSkeletalMesh(WeaponData->Mesh);
			}
		}
	}
}

void AWeapon::SetWeaponIndex(int32 InIndex)
{
	WeaponIndex = InIndex;
}

FWeaponData* AWeapon::GetWeaponData()
{
	if (IsValid(WeaponDataTable) && WeaponIndex != INDEX_NONE)
	{
		return WeaponDataTable->FindRow<FWeaponData>(*FString::FromInt(WeaponIndex), TEXT("WeaponDataTable"));
	}

	return nullptr;
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
