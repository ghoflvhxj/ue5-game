// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TestGame/TestGame.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "SkillSubsystem.h"
#include "MAbility.generated.h"

class UGameplayAbility;
class AWeapon;
class AMCharacter;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct TESTGAME_API FMAbilityBindInfo
{
	GENERATED_BODY();

public:
	FMAbilityBindInfo()
		: GameplayTag()
	{

	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GameplayTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActivate;
};

//USTRUCT(BlueprintType)
//struct TESTGAME_API FAbilityInfo
//{
//	GENERATED_BODY()
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	FText Name;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	FText Description;
//};
//
//USTRUCT(BlueprintType)
//struct TESTGAME_API FAbilityTableRow : public FTableRowBase
//{
//	GENERATED_BODY()
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	int32 Index = 0;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	FAbilityInfo AbilityInfo;
//
//public:
//	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override
//	{
//		Super::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
//		if (InRowName.ToString().IsNumeric())
//		{
//			Index = FCString::Atoi(*InRowName.ToString());
//		}
//		else
//		{
//			UE_LOG(LogTemp, Warning, TEXT("DataTable(%s) rowname(%s) is not numeric. Failed to initialize index property!!!"), *InDataTable->GetName(), *InRowName.ToString());
//		}
//	}
//};

UCLASS()
class TESTGAME_API UMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMAbilityBindInfo> Abilities;

	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent) const;
	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const;
	void ClearAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const;
};

USTRUCT(BlueprintType)
struct TESTGAME_API FTableRow_MCharacterToAbilitySet : public FTableRowBase
{
	GENERATED_BODY()

	FTableRow_MCharacterToAbilitySet()
		: FTableRowBase()
	{}


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AMCharacter>, UMAbilityDataAsset*> MCharacterClassToAbilityDataAsset;
};

UCLASS()
class TESTGAME_API UGameplayAbility_MoveToMouse : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_MoveToMouse();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UFUNCTION()
	void MoveToMouse(FGameplayEventData Payload);
};

UCLASS()
class TESTGAME_API UGameplayAbility_WeaponBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;\
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	void ClearCachedData();
protected:
	bool bClearCacheIfEnd = true;

protected:
	FVector GetCharacterLocation(bool bIncludeCapsuleHeight);
	FRotator GetCharacterRotation();
	
protected:
	TWeakObjectPtr<AMCharacter> Character = nullptr;
	TWeakObjectPtr<AWeapon> Weapon = nullptr;
	FDelegateHandle WeaponChangedDelegateHandle;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Combo : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Combo();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_BasicAttackStop : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_BasicAttackStop();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);
};


struct FSkillTableRow;

UCLASS()
class TESTGAME_API UGameplayAbility_Skill : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Skill();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UFUNCTION(BlueprintPure)
	float GetSkillParam(FGameplayTag GameplayTag);
	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetSkillInfo"))
	FSkillTableRow BP_GetSkillInfo();
	FSkillTableRow* GetSkillInfo();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* SkillTable = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SkillIndex = INDEX_NONE;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	AMCharacter* Character = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEndInstantly = true;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
};

UCLASS()
class TESTGAME_API UGameplayAbility_CollideDamage : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CollideDamage();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnCollide(AActor* OverlappedActor, AActor* OtherActor);

public:
	UFUNCTION(BlueprintCallable)
	void SetDamage(float InDamage) { Damage = InDamage; }
protected:
	float Damage = 0.f;
	FGameplayTag CueTag = FGameplayTag::EmptyTag;
};

UCLASS()
class TESTGAME_API UGameplayAbility_DamageImmune : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_DamageImmune();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	FTimerHandle TimerHandle;
	TWeakObjectPtr<const AActor> CachedInstigator = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_KnockBack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_KnockBack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void KnockBack(AActor* OverlappedActor, AActor* OtherActor);
protected:
	UPROPERTY(BlueprintReadWrite)
	float Radius = 100.f;
	UPROPERTY(BlueprintReadWrite)
	float Strength = 0.f;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Move : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Move();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetForward(float InFoward) { Foward = InFoward; }
	UFUNCTION(BlueprintCallable)
	void SetStrafe(float InStrafe) { Strafe = InStrafe; }
protected:
	float Foward = 0.f;
	float Strafe = 0.f;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Move_KeepBasicAttack : public UGameplayAbility_Move
{
	GENERATED_BODY()

public:
	UGameplayAbility_Move_KeepBasicAttack();
};

UCLASS()
class TESTGAME_API UGameplayAbility_CameraShake : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CameraShake();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;
	TArray<TWeakObjectPtr<APlayerController>> TargetPlayers;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Reload : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Reload();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void OnMontageFinished();
};

UCLASS()
class TESTGAME_API UGameplayAbility_Dash : public UGameplayAbility_Skill
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void OnMontageFinished();
};

UCLASS()
class TESTGAME_API UGameplayAbility_SpinalReflex : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_SpinalReflex();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

// 피격 시 주변 공격
UCLASS()
class TESTGAME_API UGameplayAbility_CounterAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CounterAttack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> CounterAttackClass = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_DamageToOne : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_DamageToOne();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};