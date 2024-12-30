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

	float AttackPower = Source->GetNumericAttribute(UMAttributeSet::GetAttackPowerAttribute());
	float Damage = AttackPower - (FMath::FRand() * AttackPower / 5.f);
	Damage = FMath::Max(0.f, Damage);
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UMAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -Damage));
}
