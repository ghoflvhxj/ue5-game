#pragma once

#include "AttackAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MEffect.h"

DECLARE_LOG_CATEGORY_CLASS(LogAttack, Log, Log);

FGameplayTag LightAttack = FGameplayTag::RequestGameplayTag("Action.Attack.Light");

UGameplayAbility_AttackBase::UGameplayAbility_AttackBase()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.Block"));
}

bool UGameplayAbility_AttackBase::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return Weapon->IsAttackable();
	}

	return false;
}

void UGameplayAbility_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ComboDelegateHandle.IsValid() == false)
	{
		ComboDelegateHandle = Weapon->OnComboChangedEvent.AddUObject(this, &UGameplayAbility_BasicAttack::MontageJumpToComboSection);
	}

	Weapon->SetAttackMode(AbilityTags.GetByIndex(0));
}

void UGameplayAbility_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (Weapon.IsValid())
	{
		Weapon->OnComboChangedEvent.Remove(ComboDelegateHandle);
		Weapon->ResetCombo();
		ComboDelegateHandle.Reset();
	}

	Weapon->SetAttackMode(FGameplayTag::EmptyTag);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGameplayAbility_AttackBase::PlayAttackMontage()
{
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetAttackSpeedAttribute());
	}

	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Attack", Montage, AttackSpeed, NAME_None, true, 1.f, 0.f);
			PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_BasicAttack::OnMontageFinished);

			PlayMontageTask->ReadyForActivation();
			return true;
		}
	}

	return false;
}

void UGameplayAbility_AttackBase::MontageJumpToComboSection(int32 InComboIndex)
{
	if (Character.IsValid() == false)
	{
		return;
	}

	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			FString ComboName = InComboIndex == INDEX_NONE ? TEXT("End") : FString::Printf(TEXT("Combo%d"), InComboIndex);
			
			if (Montage->IsValidSectionName(*ComboName) == false)
			{
				UE_LOG(LogAttack, Warning, TEXT("%s is invalid montage section name. Jump to last section"), *ComboName);
				ComboName = Montage->GetSectionName(Montage->GetNumSections() - 1).ToString();
			}

			if (AnimInstance->Montage_IsPlaying(Montage) && AnimInstance->Montage_GetCurrentSection(Montage) != ComboName)
			{
				AnimInstance->Montage_JumpToSection(*ComboName, Montage);
			}
		}
	}
}

void UGameplayAbility_AttackBase::OnMontageFinished()
{
	UE_LOG(LogAttack, Warning, TEXT("OnMontageFinished"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UGameplayAbility_BasicAttack::UGameplayAbility_BasicAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(LightAttack);
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.DashLight")); // 공격 어빌리티가 시작되면 다른 공격 어빌리티는 모두 취소 되도록 해야함

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
}

void UGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Character.Move.Block"));
	UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(Character.Get(), TagContainer);
	bMoveBlockReleased = false;

	TimerHandle = Character->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(GetWorld(), [this, TagContainer]() {
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(Character.Get(), TagContainer);
		bMoveBlockReleased = true;
	}));

	if (const FWeaponData* WeaponData = Weapon->GetItemData())
	{
		// 임시작업. 무기마다 다른 이펙트가 들어갈 수 있음.
		if (WeaponData->WeaponType == EWeaponType::Gun)
		{
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_AddMoveSpeed::StaticClass());
			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, EffectParamTag, -300.f);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
		}

		if (WeaponData->bBlockMovementRotate)
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->bOrientRotationToMovement = false;
			}
		}
	}

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (Weapon.IsValid())
	{
		if (const FWeaponData* WeaponData = Weapon->GetItemData())
		{
			if (WeaponData->WeaponType == EWeaponType::Gun)
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_AddMoveSpeed::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, EffectParamTag, 300.f);
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
			}
		}
	}

	if (Character.IsValid())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->bOrientRotationToMovement = true;
		}
	}

	if (bMoveBlockReleased == false)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Character.Move.Block"));
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(Character.Get(), TagContainer);
		bMoveBlockReleased = true;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UGameplayAbility_DashLightAttack::UGameplayAbility_DashLightAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.DashLight"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash")); // Character.Dash
}

void UGameplayAbility_DashLightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

UGameplayAbility_LightChargeAttack::UGameplayAbility_LightChargeAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.ChargeLight"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(LightAttack);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_LightChargeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_LightChargeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (Character.IsValid())
	{
		Character->FinishCharge();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
