// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/Bullet/Bullet.h" // DamageGiveActor 참조용
#include "Weapon.generated.h"

class UMAbilityDataAsset;
class UMActionDataAsset;
class UAttributeSet;
class AMCharacter;
struct FGameItemTableRow;
struct FOnAttributeChangeData;

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
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool LookMouseWhenCharging = true;
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

	// 캐릭터 이동 시 회전 막음
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlockMovementRotate = false;

	// 어트리뷰트 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMActionDataAsset* ActionData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMAbilityDataAsset* AbilitiesDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> Attributes = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Combo = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponData WeaponData;
};

UCLASS()
class TESTGAME_API AWeapon : public ADamageGiveActor
{
	GENERATED_BODY()

public:
	AWeapon();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMActionComponent* ActionComponent = nullptr;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//class UAbilitySystemComponent* AbilitySystemComponent = nullptr;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
protected:
	virtual void BeginPlay() override;

public:
	TSubclassOf<UAttributeSet> GetAttributeSet();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* WeaponDataTable;

public:
	void SetEquipActor(AActor* EquipActor);
	void OnEquipmentChanged(AActor* OldWeapon, AActor* NewWeapon);
	void ChangeWeaponScale(const FOnAttributeChangeData& AttributeChangeData);
protected:
	FDelegateHandle EquipChangedHandle;

	// WeaponIndex
public:
	void SetWeaponIndex(int32 ItemIndex);
	const FWeaponData* GetWeaponData() const;
	const FGameItemTableRow* GetItemTableRow() const;
public:
	UFUNCTION()
	void OnRep_WeaponIndex();
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponIndex)
	int32 WeaponIndex = INDEX_NONE;

public:
	bool GetMuzzleTransform(FTransform& OutTransform);

	// 공격 모드(약, 강, 대쉬약, 대쉬강 등등)
public:
	void SetAttackMode(FGameplayTag InAttackMode);
protected:
	FGameplayTag AttackMode;

	// 콤보
public:
	virtual void NextCombo();
	virtual void ResetCombo();
	virtual void OnAttackCoolDownFinished();
public:
	bool IsComboableWeapon() const;
	bool IsComboable() const;
	int32 GetCombo() const { return Combo; }
public:
	DECLARE_EVENT_OneParam(AWeapon, FOnComboChangedEvent, int32);
	FOnComboChangedEvent OnComboChangedEvent;
protected:
	mutable int32 Combo = INDEX_NONE;
	mutable FGameplayTag ComboActionTag;

public:
	bool IsAttackable() const;
	bool IsCoolDown() const;
	bool IsAttacking() const;
protected:
	bool bCoolDown = false;

public:
	DECLARE_EVENT_OneParam(AWeapon, FOnChargeChangedEvent, bool);
	FOnChargeChangedEvent& GetOnChargeChangedEvent() { return OnChargeChangedEvent; }
public:
	bool IsCharged() { return FMath::IsNearlyEqual(ChargeValue, 1.f); }
	void Charge();
	void UnCharge();
protected:
	float ChargeValue = 0.f;
	FOnChargeChangedEvent OnChargeChangedEvent;

	FTimerHandle AttackTimerHandle;

// 임시. WeaponTable만들고 Info와 Data를 가리키도록 변경해야 함
public:
	FWeaponInfo WeaponInfo;
};

UCLASS()
class TESTGAME_API ADelegator : public AActor
{
	GENERATED_BODY()

public:
	ADelegator();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAbilitySystemComponent* AbilitySystemComponent = nullptr;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
};