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

void AWeapon::OnEquipped(AActor* EquipActor)
{
	if (IsValid(EquipActor))
	{
		if (FWeaponData* WeaponData = GetWeaponData())
		{
			FString SocketName;
			switch (WeaponData->WeaponType)
			{
				default:
				case EWeaponType::Sword:
				{
					SocketName = TEXT("hand_rSocket");
				}
				break;
				case EWeaponType::Gun:
				{
					SocketName = TEXT("hand_rSocket_FPS");
				}
				break;
			}

			AttachToActor(EquipActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), *SocketName);
		}
	}

	SetOwner(EquipActor);
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

bool AWeapon::GetMuzzleTransform(FTransform& OutTransform)
{
	if (USkeletalMeshComponent* WeaponMesh = GetComponentByClass<USkeletalMeshComponent>())
	{
		OutTransform = WeaponMesh->GetSocketTransform(TEXT("Muzzle"));
		return true;
	}

	return false;
}

bool AWeapon::BasicAttack()
{
	if (IsAttackable() == false || IsValid(GetOwner()) == false)
	{
		return false;
	}

	if (FWeaponData* WeaponData = GetWeaponData())
	{
		if (WeaponData->AttackSpeed > 0.f)
		{
			bAttackable = false;
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AWeapon::OnAttackCoolDownFinished, WeaponData->AttackSpeed, false);
		}

		return true;
	}

	return false;
}

void AWeapon::OnAttackCoolDownFinished()
{
	bAttackable = true;
}

bool AWeapon::IsAttackable() const
{
	return bAttackable;
}
