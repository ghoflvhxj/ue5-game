// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.generated.h"

enum class WeaponRotateType
{
	None,
	Instantly,
	Smoothly,
};

enum class WeaponAttackType
{
	None,
	Click,
	Press,
	Charge
};

struct FWeaponData
{
	float AttackSpeed = 1.f;
};

struct FWeaponControlData
{
	WeaponRotateType RotateType;
	WeaponAttackType AttackType;
};

UCLASS()
class TESTGAME_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMActionComponent* ActionComponent;

public:
	virtual bool Attack();
	virtual void OnAttackCoolDownFinished();

public:
	bool IsAttackable() const;
protected:
	bool bAttackable = true;

	float AttackSpeed = 0.25f;
	FTimerHandle AttackTimerHandle;
};
