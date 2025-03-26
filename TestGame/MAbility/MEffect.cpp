#pragma once

#include "MEffect.h"
#include "GameplayTags.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "MCalculation.h"

DECLARE_LOG_CATEGORY_CLASS(LogEffect, Log, Log);

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	FGameplayEffectExecutionDefinition EffectExeutionDef;
	EffectExeutionDef.CalculationClass = UMGameplayEffectExecutionCalculation_Damage::StaticClass();
	Executions.Add(EffectExeutionDef);

	FGameplayEffectCue EffectCue;
	EffectCue.GameplayCueTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayCue.UI.Floater.Deal"));
	//EffectCue.GameplayCueTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Default")); // 이펙트 경우는 주체가 무엇이냐에 따라 나올 이펙트가 다른 경우가 많음. 서브 클래스에서 설정해줘야 함.
	EffectCue.MagnitudeAttribute = UMAttributeSet::GetHealthAttribute();
	GameplayCues.Add(EffectCue);
}

void UGameplayEffect_Damage::UpdateCueParams(AActor* InCauser, AActor* InTarget, FGameplayCueParameters& InParam)
{
	if (CueLocationRule > 0)
	{
		InParam.Location = InTarget->GetActorLocation();

		if (USkeletalMeshComponent* SkeletalMeshComponent = InTarget->GetComponentByClass<USkeletalMeshComponent>())
		{
			InParam.Location = SkeletalMeshComponent->GetSocketLocation("FX_Root");
		}
	}
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

UGameplayEffect_AddAttackPower::UGameplayEffect_AddAttackPower()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetAttackPowerAttribute();
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

UGameplayEffect_AddMaxHealth::UGameplayEffect_AddMaxHealth()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetMaxHealthAttribute();
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

UGameplayEffect_AddProjectileScale::UGameplayEffect_AddProjectileScale()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetProjectileScaleAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
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

UGameplayEffect_SetMoveSpeed::UGameplayEffect_SetMoveSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	ModifierInfo.Attribute = Attribute = UMAttributeSet::GetMoveSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	SetByCaller.DataTag = GetEffectValueTag();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}
