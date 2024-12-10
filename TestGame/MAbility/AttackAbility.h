#pragma once

#include "MAbility.h"
#include "AttackAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class UGameplayAbility_AttackBase : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

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