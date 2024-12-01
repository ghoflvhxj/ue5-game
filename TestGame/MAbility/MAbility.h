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

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GameplayTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActivate;

	// server, client, all
};

UCLASS()
class TESTGAME_API UMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMAbilityBindInfo> Abilities;

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
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	void ClearCachedData();
	
protected:
	TWeakObjectPtr<AMCharacter> CachedCharacter = nullptr;
	TWeakObjectPtr<AWeapon> CachedWeapon = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_BasicAttack : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_BasicAttack();

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
public:
	void SetCombo(int32 InComboIndex);
	void FinishAttack();
	UFUNCTION()
	void OnMontageFinished();

protected:
	FDelegateHandle ComboDelegateHandle;
	FTimerHandle TimerHandle;
	class UAbilityTask_PlayMontageAndWait* PlayMontageTask = nullptr;

	float WeaponAttackSpeed = 1.f;
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
class UGameplayAbility_Skill : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Skill();

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

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
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
};

UCLASS()
class UGameplayAbility_CollideDamage : public UGameplayAbility
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
};

UCLASS()
class UGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Damage();
};

UCLASS()
class UGameplayAbility_DamageImmune : public UGameplayAbility
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
class UGameplayAbility_KnockBack : public UGameplayAbility
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
class UGameplayAbility_Move : public UGameplayAbility
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
class UGameplayAbility_Move_KeepBasicAttack : public UGameplayAbility_Move
{
	GENERATED_BODY()

public:
	UGameplayAbility_Move_KeepBasicAttack();
};

UCLASS()
class UGameplayAbility_CameraShake : public UGameplayAbility
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
class UGameplayAbility_Reload : public UGameplayAbility_WeaponBase
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

/* 사격 후 탄약 감소 */
UCLASS()
class UGameplayEffect_ConsumeAmmo : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_ConsumeAmmo();
};

/* 리로드 */
UCLASS()
class UGameplayEffect_Reload : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Reload();
};

UCLASS()
class UGAmeplayEffect_AddMoveSpeed : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGAmeplayEffect_AddMoveSpeed();
};