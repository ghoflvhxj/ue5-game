// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MWeapon/Weapon.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "AbilitySystemBlueprintLibrary.h"

// 임시
#include "TestGame/MPlayerController/MPlayerController.h"

DECLARE_LOG_CATEGORY_CLASS(LogAbility, Log, Log);

void UMAbilityDataAsset::GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const
{ 
	if (AbilitySystemComponent->IsNetSimulating())
	{
		return;
	}

	for (const FMAbilityBindInfo& BindInfo : Abilities)
	{
		if (IsValid(BindInfo.GameplayAbilityClass) == false)
		{
			continue;
		}

		FGameplayTag GameplayTag = BindInfo.GameplayTag;
		if (Handles.Contains(GameplayTag))
		{
			AbilitySystemComponent->ClearAbility(Handles[GameplayTag]);
		}
		
		Handles.Emplace(GameplayTag, AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass, -1, -1)));
		if (BindInfo.bActivate && Handles.Contains(GameplayTag))
		{
			AbilitySystemComponent->TryActivateAbility(Handles[GameplayTag], true);
		}
	}
}

void UMAbilityDataAsset::ClearAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const
{
	for (const FMAbilityBindInfo& BindInfo : Abilities)
	{
		if (Handles.Contains(BindInfo.GameplayTag))
		{
			AbilitySystemComponent->ClearAbility(Handles[BindInfo.GameplayTag]);
			Handles.Remove(BindInfo.GameplayTag);
		}
	}
}

UGameplayAbility_MoveToMouse::UGameplayAbility_MoveToMouse()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.Move")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Dead")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.BasicAttack")));
}

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	if (APlayerController* Controller = UGameplayStatics::GetPlayerController(GetAvatarActorFromActorInfo(), 0))
	{
		FHitResult HitResult;
		Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

		if (HitResult.bBlockingHit)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, HitResult.Location);
			DrawDebugSphere(GetWorld(), HitResult.Location, 100.f, 32, FColor::Green);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGameplayAbility_MoveToMouse::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	if (APlayerController* Controller = UGameplayStatics::GetPlayerController(GetAvatarActorFromActorInfo(), 0))
	{
		Controller->StopMovement();
	}
}

void UGameplayAbility_MoveToMouse::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UGameplayAbility_MoveToMouse::MoveToMouse(FGameplayEventData Payload)
{
	const APawn* Pawn = Cast<APawn>(Payload.Target);
	if (IsValid(Pawn) == false)
	{
		return;
	}
	
	if (AMPlayerControllerInGame* Controller = Cast<AMPlayerControllerInGame>(Pawn->GetController()))
	{
		FHitResult HitResult;
		Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

		if (HitResult.bBlockingHit)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, HitResult.Location);
			DrawDebugSphere(GetWorld(), HitResult.Location, 100.f, 32, FColor::Green);
		}
	}
}

UGameplayAbility_BasicAttack::UGameplayAbility_BasicAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.BasicAttack"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float BasicAttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		BasicAttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetBasicAttackSpeedAttribute());
	}

	AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo());

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Character.Move.Block"));
	UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(Character, TagContainer);

	TimerHandle = Character->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(Character, [this, Character, TagContainer]() {
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(Character, TagContainer);
	}));

	CachedWeapon = Character->GetEquipItem<AWeapon>();
	if (CachedWeapon && CachedWeapon->GetItemData()->WeaponType == EWeaponType::Gun)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGAmeplayEffect_AddMoveSpeed::StaticClass());
		EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.MoveSpeed"), -300.f);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
	}

	Character->OnWeaponChangedEvent.AddWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo](AActor* Old, AActor* New) {
		if (Old == CachedWeapon)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	});

	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "BasicAttack", Montage, BasicAttackSpeed, NAME_None, true, 1.f, 0.f);
			PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_BasicAttack::OnMontageFinished);

			PlayMontageTask->ReadyForActivation();
			return;
		}
	}


	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_BasicAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	if (CachedWeapon)
	{
		CachedWeapon->FinishBasicAttack();

		if (const FWeaponData* WeaponData = CachedWeapon->GetItemData())
		{
			if (WeaponData->WeaponType == EWeaponType::Gun)
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGAmeplayEffect_AddMoveSpeed::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.MoveSpeed"), 300.f);
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
			}
		}
	}

	CachedWeapon = nullptr;
	//UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UGameplayAbility_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (CachedWeapon)
	{
		CachedWeapon->FinishBasicAttack();

		if (const FWeaponData* WeaponData = CachedWeapon->GetItemData())
		{
			if (WeaponData->WeaponType == EWeaponType::Gun)
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGAmeplayEffect_AddMoveSpeed::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.MoveSpeed"), 300.f);
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
			}
		}
	}

	CachedWeapon = nullptr;

	//UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

bool UGameplayAbility_BasicAttack::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */)
{
	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			return Weapon->IsAttackable() && Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
		}
	}

	return false;
}

void UGameplayAbility_BasicAttack::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			ComboDelegateHandle = Weapon->OnComboChangedEvent.AddUObject(this, &UGameplayAbility_BasicAttack::SetCombo);
		}
	}
}

void UGameplayAbility_BasicAttack::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			Weapon->OnComboChangedEvent.Remove(ComboDelegateHandle);
		}
	}
}

void UGameplayAbility_BasicAttack::SetCombo(int32 InComboIndex)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (IsValid(Avatar) == false)
	{
		return;
	}

	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			FString ComboName = FString::Printf(TEXT("Combo%d"), InComboIndex);
			if (AnimInstance->Montage_IsPlaying(Montage) && Montage->IsValidSectionName(*ComboName))
			{
				AnimInstance->Montage_JumpToSection(*ComboName, Montage);
			}
		}
	}
}

void UGameplayAbility_BasicAttack::FinishAttack()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (IsValid(Avatar) == false)
	{
		return;
	}

	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			if (AnimInstance->Montage_IsPlaying(Montage) && Montage->IsValidSectionName("End"))
			{
				AnimInstance->Montage_JumpToSection("End", Montage);
			}
		}
	}
}

void UGameplayAbility_BasicAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UGameplayAbility_Skill::UGameplayAbility_Skill()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Controller.MouseLeftClick");
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Skill"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Dead")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Move"));
}

bool UGameplayAbility_Skill::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		if (FSkillTableRow* SkillInfo = GetSkillInfo())
		{
			for (const FBuffInfo& BuffInfo : SkillInfo->BuffInfos)
			{
				if (IsValid(BuffInfo.EffectClass) == false)
				{
					continue;
				}

				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BuffInfo.EffectClass);

				TArray<FGameplayTag> GameplayTags;
				BuffInfo.TagToValue.GenerateKeyArray(GameplayTags);

				SpecHandle.Data->SetByCallerTagMagnitudes = SpecHandle.Data->SetByCallerTagMagnitudes.FilterByPredicate([&GameplayTags](TPair<FGameplayTag, float> Element) {
					return GameplayTags.Contains(Element.Key);
				});

				for (const TPair<FGameplayTag, float>& TagToValuePair : BuffInfo.TagToValue)
				{
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, TagToValuePair.Key, TagToValuePair.Value);
				}

				if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
				{
					ActiveEffectHandles.Add(AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
				}
			}	
		}
		return true;
	}

	return false;
}

float UGameplayAbility_Skill::GetSkillParam(FGameplayTag GameplayTag)
{
	if (FSkillTableRow * SkillTableRow = GetSkillInfo())
	{
		if (SkillTableRow->Params.Contains(GameplayTag))
		{
			return SkillTableRow->Params[GameplayTag];
		}
	}

	return 0.f;
}

FSkillTableRow UGameplayAbility_Skill::BP_GetSkillInfo()
{
	if (FSkillTableRow* SkillTableRow = GetSkillInfo())
	{
		return *SkillTableRow;
	}

	return FSkillTableRow();
}

FSkillTableRow* UGameplayAbility_Skill::GetSkillInfo()
{
	if (IsValid(SkillTable))
	{
		return SkillTable->FindRow<FSkillTableRow>(*FString::Printf(TEXT("%d"), SkillIndex), TEXT("SkillTable"));
	}

	return nullptr;
}

UGameplayAbility_CollideDamage::UGameplayAbility_CollideDamage()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateNo;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
}

void UGameplayAbility_CollideDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	 
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
	}

	if (AActor* AbilityOwer = GetAvatarActorFromActorInfo())
	{
		AbilityOwer->OnActorBeginOverlap.AddDynamic(this, &UGameplayAbility_CollideDamage::OnCollide);
	}

	if (UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo())
	{
		bool bFound = false;
		float NewDamage = AbilityComponent->GetGameplayAttributeValue(UMWeaponAttributeSet::GetAttackPowerAttribute(), bFound);
		if (bFound)
		{
			Damage = NewDamage;
			AbilityComponent->GetGameplayAttributeValueChangeDelegate(UMWeaponAttributeSet::GetAttackPowerAttribute()).AddWeakLambda(this, [this](const FOnAttributeChangeData& AttributeChangeData) {
				Damage = AttributeChangeData.NewValue;
			});
		}

	}
}

void UGameplayAbility_CollideDamage::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AActor* AbilityOwer = GetAvatarActorFromActorInfo())
	{
		if (AbilityOwer->OnActorBeginOverlap.IsAlreadyBound(this, &UGameplayAbility_CollideDamage::OnCollide))
		{
			AbilityOwer->OnActorBeginOverlap.RemoveDynamic(this, &UGameplayAbility_CollideDamage::OnCollide);
		}
	}
}

void UGameplayAbility_CollideDamage::OnCollide(AActor* OverlappedActor, AActor* OtherActor)
{
	if (IsValid(OtherActor) == false)
	{
		return;
	}

	ACharacter* MyCharacter = nullptr;
	AActor* Owner = GetAvatarActorFromActorInfo();
	while (IsValid(Owner))
	{
		if (Owner->IsA<ACharacter>())
		{
			MyCharacter = Cast<ACharacter>(Owner);
			break;
		}
		Owner = Owner->GetOwner();
	}

	if (IsValid(MyCharacter) && HasAuthority(&CurrentActivationInfo))
	{
		//UE_LOG(LogTemp, Warning, TEXT("OnCollide %s"), *OtherActor->GetActorLabel());

		if (AMCharacter* OtherCharacter = Cast<AMCharacter>(OtherActor))
		{
			if (OtherCharacter->IsDead() == false && OtherCharacter->IsPlayerControlled() != MyCharacter->IsPlayerControlled())
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_Damage::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.Damage"), -Damage);

				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OtherActor));
			}
		}
	}
}

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

UGameplayAbility_DamageImmune::UGameplayAbility_DamageImmune()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateNo;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Event.Damaged");
	AbilityTriggers.Add(TriggerData);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Ability.DamageImmune"));
}

void UGameplayAbility_DamageImmune::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		if (TriggerEventData != nullptr)
		{
			CachedInstigator = TriggerEventData->Instigator.Get();
		}

		// 태스크에 1000횟수를 넣어도, 어빌리티가 3초 뒤에 끝나기 때문에, 0.1초가 30번 하면 태스크도 끝남
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}), 3.f, false);
		}
	}
}

void UGameplayAbility_DamageImmune::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AActor* Actor = GetAvatarActorFromActorInfo())
	{
		//UE_LOG(LogTemp, Warning, TEXT("DamageImmune::EndAbility Clearoverlap "));

		//Actor->ClearComponentOverlaps();
		if (CachedInstigator.IsValid())
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			Actor->GetComponents(PrimitiveComponents);
			for (UPrimitiveComponent* const PrimComp : PrimitiveComponents)
			{
				TArray<FOverlapInfo> OverlapInfos = PrimComp->GetOverlapInfos();
				for (FOverlapInfo& OverlapInfo : OverlapInfos)
				{
					if (OverlapInfo.OverlapInfo.GetActor() == CachedInstigator)
					{
						PrimComp->EndComponentOverlap(OverlapInfo);
					}
				}
			}
		}
		Actor->UpdateOverlaps();
	}
}

UGameplayAbility_KnockBack::UGameplayAbility_KnockBack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateNo;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
}

void UGameplayAbility_KnockBack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		if (AActor* AbilityOwer = GetAvatarActorFromActorInfo())
		{
			AbilityOwer->OnActorBeginOverlap.AddDynamic(this, &UGameplayAbility_KnockBack::KnockBack); 
		}
	}
}

void UGameplayAbility_KnockBack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AActor* AbilityOwer = GetAvatarActorFromActorInfo())
	{
		if (AbilityOwer->OnActorBeginOverlap.IsAlreadyBound(this, &UGameplayAbility_KnockBack::KnockBack))
		{
			AbilityOwer->OnActorBeginOverlap.RemoveDynamic(this, &UGameplayAbility_KnockBack::KnockBack);
		}
	}
}

void UGameplayAbility_KnockBack::KnockBack(AActor* OverlappedActor, AActor* OtherActor)
{
	AActor* Owner = GetAvatarActorFromActorInfo();
	if (IsValid(Owner) == false)
	{
		return;
	}

	if (OtherActor == Owner)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (Character->IsPlayerControlled())
		{
			return;
		}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->AddRadialImpulse(Owner->GetActorLocation(), Radius, Strength, ERadialImpulseFalloff::RIF_Linear, false);
		}
	}
}

UGameplayAbility_CameraShake::UGameplayAbility_CameraShake()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Event.Damaged");
	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_CameraShake::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		for (TWeakObjectPtr<APlayerController> PlayerController : TargetPlayers)
		{
			if (PlayerController.IsValid())
			{
				PlayerController->ClientStartCameraShake(CameraShakeClass);
			}
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

bool UGameplayAbility_CameraShake::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(Character) == false)
	{
		return false;
	}

	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(Character))
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (APlayerController * PlayerController = PlayerState->GetPlayerController())
			{
				if (PlayerController->GetViewTarget() == Character)
				{
					TargetPlayers.Add(PlayerController);
				}
			}
		}
	}

	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

void UGameplayAbility_CameraShake::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);

	TargetPlayers.Empty();
}

UGameplayAbility_Move::UGameplayAbility_Move()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Controller.Move");
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Move")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Dead")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Move.Block")));
	
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
}

void UGameplayAbility_Move::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->AddMovementInput(FVector(Foward, Strafe, 0.f));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UGameplayAbility_Move_KeepBasicAttack::UGameplayAbility_Move_KeepBasicAttack()
{
	CancelAbilitiesWithTag.RemoveTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
}

UGameplayAbility_Combo::UGameplayAbility_Combo()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.BasicAttack.Combo"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); 
		return; 
	}

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Weapon);
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_ConsumeAmmo::StaticClass());
			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.ConsumeMagazine"), -1);
			ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, EffectSpecHandle, TargetDataHandle);

			Weapon->OnAttacked();
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGameplayAbility_Combo::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			if (Weapon->IsAttackable())
			{
				return true;
			}
			else if(UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
			{
				AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_BasicAttackStop::StaticClass(), true);
				return false;
			}
		}
	}

	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

UGameplayAbility_BasicAttackStop::UGameplayAbility_BasicAttackStop()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack.Finish"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack.Finish"));
}

void UGameplayAbility_BasicAttackStop::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TArray<FGameplayAbilitySpecHandle> ActiveAbilities;
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->FindAllAbilitiesWithTags(ActiveAbilities, FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Action.BasicAttack")));
		for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : ActiveAbilities)
		{
			FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
			for (UGameplayAbility* Abilitiy : AbilitySpec->GetAbilityInstances())
			{
				if (UGameplayAbility_BasicAttack* BasicAttackAbility = Cast<UGameplayAbility_BasicAttack>(Abilitiy))
				{
					BasicAttackAbility->FinishAttack();
				}
			}
		}
	}

	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.BasicAttack")))
		{
			if (AnimInstance->Montage_IsPlaying(Montage))
			{
				AnimInstance->OnMontageEnded.AddDynamic(this, &UGameplayAbility_BasicAttackStop::OnMontageEnd);
				return;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameplayAbility_BasicAttackStop::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* AttackMontage = ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.BasicAttack")))
		{
			if (AttackMontage == Montage)
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}
		}

		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UGameplayAbility_BasicAttackStop::OnMontageEnd);
	}
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

UGameplayAbility_Reload::UGameplayAbility_Reload()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.BasicAttack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float ReloadSpeed = 1.f;

	if (AActor* AbilityOwner = GetAvatarActorFromActorInfo())
	{
		if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
		{
			if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
			{
				UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "BasicAttack", Montage, ReloadSpeed, NAME_None, true, 1.f, 0.f);
				PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Reload::OnMontageFinished);
				PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Reload::OnMontageFinished);

				PlayMontageTask->ReadyForActivation();
				return;
			}
		}
	}
}

void UGameplayAbility_Reload::OnMontageFinished()
{
	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			UE_CLOG(Character->IsNetMode(NM_DedicatedServer), LogTemp, Warning, TEXT("Server Reload"));
			UE_CLOG(Character->IsNetMode(NM_Client), LogTemp, Warning, TEXT("Client Reload"));
			FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Weapon);
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_Reload::StaticClass());
			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.Ammo"), 30.f);
			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.TotalAmmo"), -30.f);
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UGAmeplayEffect_AddMoveSpeed::UGAmeplayEffect_AddMoveSpeed()
{
	FGameplayModifierInfo ModifierInfo;
	FSetByCallerFloat SetByCaller;

	// 탄약 충전
	ModifierInfo.Attribute = UMAttributeSet::GetMoveSpeedAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Attribute.MoveSpeed");
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}
