#pragma once

#include "MEffect.h"
#include "GameplayTags.h"
#include "TestGame/MAttribute/MAttribute.h"

DECLARE_LOG_CATEGORY_CLASS(LogEffect, Log, Log);

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Attribute.Damage");
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	ModifierInfo.TargetTags.IgnoreTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Ability.DamageImmune"));
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_ConsumeAmmo::UGameplayEffect_ConsumeAmmo()
{
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMWeaponAttributeSet::GetAmmoAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Attribute.ConsumeAmmo");
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
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Attribute.Ammo");
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);

	// 전체 탄약 감소
	ModifierInfo.Attribute = UMWeaponAttributeSet::GetTotalAmmoAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Attribute.TotalAmmo");
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddMoveSpeed::UGameplayEffect_AddMoveSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetMoveSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = EffectParamTag;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}

UGameplayEffect_AddAttackSpeed::UGameplayEffect_AddAttackSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetAttackSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = EffectParamTag;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}
