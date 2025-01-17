#pragma once

#include "MAbility.h"
#include "AttackAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class ABullet;

UCLASS()
class TESTGAME_API UGameplayAbility_AttackBase : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_AttackBase();

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	virtual bool PlayAttackMontage();
	virtual void MontageJumpToComboSection(int32 InComboIndex);
	UFUNCTION()
	virtual void OnMontageFinished();

protected:
	FDelegateHandle ComboDelegateHandle;
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = nullptr;

public:
	FGameplayTag GetLightAttackTag()
	{
		return FGameplayTag::RequestGameplayTag("Action.Attack.Light");
	}
	FGameplayTag GetEffectValueTag()
	{
		return FGameplayTag::RequestGameplayTag("Effect.Value");
	}
};


UCLASS()
class TESTGAME_API UGameplayAbility_BasicAttack : public UGameplayAbility_AttackBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_BasicAttack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	FTimerHandle TimerHandle;

	float WeaponAttackSpeed = 1.f;

	bool bMoveBlockReleased = false;
};

UCLASS()
class UGameplayAbility_DashLightAttack : public UGameplayAbility_AttackBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_DashLightAttack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_LightChargeAttack : public UGameplayAbility_AttackBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_LightChargeAttack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_SwordWave : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_SwordWave();

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */);
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	FDelegateHandle WeaponComboChangeDelegateHandle;

protected:
	void SpawnSwordWave();
	void BindToWeaponCombo();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABullet> BulletClass = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Batto : public UGameplayAbility_LightChargeAttack
{
	GENERATED_BODY()

public:
	UGameplayAbility_Batto();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	virtual void MontageJumpToComboSection(int32 InComboIndex) override;
};