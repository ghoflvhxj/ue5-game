#pragma once

#include "MAbility.h"
#include "AttackAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class ABullet;
class UGameplayEffect_Damage;

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

public:
	float GetAttackSpeed() const;

protected:
	FDelegateHandle ComboDelegateHandle;
	FTimerHandle WeaponFinishCoolDownHandle;
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

public:
	virtual int32 GetEffectIndex() const override;
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