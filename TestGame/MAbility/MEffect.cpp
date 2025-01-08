#pragma once

#include "MEffect.h"
#include "GameplayTags.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "MCalculation.h"

DECLARE_LOG_CATEGORY_CLASS(LogEffect, Log, Log);

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	//FGameplayModifierInfo ModifierInfo;
	//ModifierInfo.Attribute = UMAttributeSet::GetHealthAttribute();
	//ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	//FSetByCallerFloat SetByCaller;
	//SetByCaller.DataTag = EffectParamTag;
	//ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	//ModifierInfo.TargetTags.IgnoreTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Ability.DamageImmune"));
	//Modifiers.Add(ModifierInfo);

	FGameplayEffectExecutionDefinition EffectExeutionDef;
	EffectExeutionDef.CalculationClass = UMGameplayEffectExecutionCalculation_Damage::StaticClass();
	Executions.Add(EffectExeutionDef);

	FGameplayEffectCue EffectCue;
	EffectCue.GameplayCueTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayCue.UI.Floater.Deal"));
	EffectCue.GameplayCueTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Default"));
	EffectCue.MagnitudeAttribute = UMAttributeSet::GetHealthAttribute();
	GameplayCues.Add(EffectCue);
}

UGameplayEffect_ConsumeAmmo::UGameplayEffect_ConsumeAmmo()
{
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMWeaponAttributeSet::GetAmmoAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_Reload::UGameplayEffect_Reload()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	// 탄약 충전
	ModifierInfo.Attribute = UMWeaponAttributeSet::GetAmmoAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);

	// 전체 탄약 감소
	ModifierInfo.Attribute = UMWeaponAttributeSet::GetTotalAmmoAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddMoveSpeed::UGameplayEffect_AddMoveSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetMoveSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddAttackSpeed::UGameplayEffect_AddAttackSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetAttackSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddWeaponScale::UGameplayEffect_AddWeaponScale()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetWeaponScaleAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddHealth::UGameplayEffect_AddHealth()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);

	FGameplayEffectCue EffectCue;
	EffectCue.GameplayCueTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayCue.UI.Floater.Heal"));
	EffectCue.MagnitudeAttribute = UMAttributeSet::GetHealthAttribute();
	GameplayCues.Add(EffectCue);
}

UGameplayEffect_SetHealth::UGameplayEffect_SetHealth()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_SetMaxHealth::UGameplayEffect_SetMaxHealth()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetMaxHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}