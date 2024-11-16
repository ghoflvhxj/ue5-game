// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Sword,
	Gun,
};

UENUM(BlueprintType)
enum class EWeaponInputType : uint8
{
	None,
	Click,
	Press,
	Charge
};

UENUM(BlueprintType)
enum class EWeaponRotateType : uint8
{
	None,
	Instantly,
	Smoothly
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::None;

	// 컨트롤 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponInputType WeaponInputType = EWeaponInputType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponRotateType WeaponRotateType = EWeaponRotateType::None;

	// 어트리뷰트 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMActionDataAsset* ActionData = nullptr;
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
	void OnEquipped(AActor* EquipActor);

public:
	UFUNCTION()
	void OnRep_WeaponIndex();
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponIndex)
	int32 WeaponIndex = INDEX_NONE;

public:
	void SetWeaponIndex(int32 ItemIndex);
	FWeaponData* GetWeaponData();

public:
	bool GetMuzzleTransform(FTransform& OutTransform);

public:
	virtual bool BasicAttack();
	virtual void OnAttackCoolDownFinished();

public:
	bool IsAttackable() const;
protected:
	bool bAttackable = true;

	FTimerHandle AttackTimerHandle;
};
