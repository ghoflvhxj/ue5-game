// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MAttribute.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAttribute, Log, All);

class UGameplayAbility;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FAttributeInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute Attribute;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Decription;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UPaperSprite* Icon = nullptr;
};

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
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, MaxHealth);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
public:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
public:
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Health);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, Health);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData AttackPower;
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, AttackPower);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, MoveSpeed);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData AttackSpeed;
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UMAttributeSet, AttackSpeed);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData KnockBackPower;
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