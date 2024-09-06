// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
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

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMActionDataAsset* ActionData = nullptr;
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
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMActionComponent* ActionComponent;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* WeaponDataTable;

public:
	UFUNCTION()
	void OnRep_WeaponIndex();
private:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponIndex)
	int32 WeaponIndex = INDEX_NONE;

public:
	void SetWeaponIndex(int32 ItemIndex);
	FWeaponData* GetWeaponData();

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
