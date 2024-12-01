// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MAttribute.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAttribute, Log, All);

class UGameplayAbility;
struct FGameplayTag;

UCLASS(DefaultToInstanced, Blueprintable)
class UMAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData SkillPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData AttackMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData BasicAttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData KnockBackPower;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, MaxHealth);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, Health);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Health);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, AttackPower);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, MoveSpeed);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, AttackMoveSpeed);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, BasicAttackSpeed);
};

UCLASS(Blueprintable)
class UMWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData KnockBackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MagazineAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData TotalAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData ConsumeAmmo;

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, AttackPower);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, AttackSpeed);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, KnockBackPower);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, Ammo);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, MagazineAmmo);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, TotalAmmo);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMWeaponAttributeSet, ConsumeAmmo);
};