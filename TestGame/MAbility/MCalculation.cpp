#pragma once

#include "MCalculation.h"
#include "GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MAttribute/MAttribute.h"

DECLARE_LOG_CATEGORY_CLASS(LogCalculation, Log, Log);

void UMGameplayEffectExecutionCalculation_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* Target = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* Source = ExecutionParams.GetSourceAbilitySystemComponent();
	if (IsValid(Target) == false || IsValid(Source) == false)
	{
		return;
	}

	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	if (Target->GetTagCount(FGameplayTag::RequestGameplayTag("Character.Ability.DamageImmune")) > 0)
	{
		return;
	}

	if (Target->GetTagCount(FGameplayTag::RequestGameplayTag("Character.Ability.DamageToOne")) > 0)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UMAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -1.f));
		return;
	}

	// 스킬은 공격력의 n%를 적용
	FGameplayEffectSpec DamageEffectSpec = ExecutionParams.GetOwningSpec();
	float DamageScale = DamageEffectSpec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("DamageParam.DamageScale"), false, 1.f);
	float AttackPowerScale = DamageEffectSpec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("DamageParam.AttackPowerScale"), false, 1.f);
	float Additive = DamageEffectSpec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("DamageParam.AdditiveDamage"), false, 0.f);
	float AttackPower = Source->GetNumericAttribute(UMAttributeSet::GetAttackPowerAttribute());
	float Damage = ((AttackPower * AttackPowerScale) + Additive) * DamageScale;
	Damage = FMath::Max(0.f, Damage);
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UMAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -Damage));
}
