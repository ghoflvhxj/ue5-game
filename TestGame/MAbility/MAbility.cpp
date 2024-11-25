// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
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

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Controller.MouseRightClick"));
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dead")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.BasicAttack")));
}

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Controller.MouseRightClick")));

	UE_LOG(LogAbility, Warning, TEXT("MoveToMouse Ability %s"), *GetName())

	if (IsValid(WaitTask))
	{
		WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_MoveToMouse::MoveToMouse);
		WaitTask->ReadyForActivation();
	}
}

void UGameplayAbility_MoveToMouse::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	UE_CLOG(GetAvatarActorFromActorInfo()->IsNetMode(NM_Client), LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));

	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (IsValid(Pawn) == false)
	{
		return;
	}

	if (AMPlayerControllerInGame* Controller = Cast<AMPlayerControllerInGame>(Pawn->GetController()))
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

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Controller.MouseLeftClick");
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Action.BasicAttack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.State.Dead"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Character.Action.Move"));
}

void UGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{	
	//if (IsValid(WaitTask) == false && TriggerEventData != nullptr)
	//{
	//	WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TriggerEventData->EventTag);
	//	WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_CharacterAction::Action);
	//	WaitTask->ReadyForActivation();
	//}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AActor* Actor = GetAvatarActorFromActorInfo())
	{
		UE_CLOG(Actor->HasAuthority(), LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
	}

	if (TriggerEventData == nullptr)
	{
		return;
	}

	if (IsValid(TriggerEventData->Instigator) && CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		float BasicAttackSpeed = 1.f;

		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			bool bAttributeFound = false;
			BasicAttackSpeed = AbilitySystemComponent->GetGameplayAttributeValue(UMAttributeSet::GetBasicAttackSpeedAttribute(), bAttributeFound);
		}

		// 각도 회전
		AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo());
		if (IsValid(Character))
		{
			Character->StartBasicAttack();
		}

		if (UMActionComponent* ActionComponent = TriggerEventData->Instigator->GetComponentByClass<UMActionComponent>())
		{
			if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
			{
				if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName(TEXT("Attack")), Montage, BasicAttackSpeed, NAME_None, false))
				{
					PlayMontageTask->ReadyForActivation();
					FTimerHandle THandle;
					TriggerEventData->Instigator->GetWorldTimerManager().SetTimer(THandle, FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]() {
						EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
					}), IsValid(Montage) ? Montage->GetPlayLength() : 1.f, false);
				}
			}
		}

		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameplayAbility_BasicAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UGameplayAbility_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetRotateToTargetAngle(false);
	}

	UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

bool UGameplayAbility_BasicAttack::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */)
{
	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags) && Character->IsAttackable();
	}

	return false;
}
UGameplayAbility_CollideDamage::UGameplayAbility_CollideDamage()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
}

void UGameplayAbility_CollideDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		if (AActor* AbilityOwer = GetAvatarActorFromActorInfo())
		{
			AbilityOwer->OnActorBeginOverlap.AddDynamic(this, &UGameplayAbility_CollideDamage::OnCollide);
		}
	}
}

void UGameplayAbility_CollideDamage::OnCollide(AActor* OverlappedActor, AActor* OtherActor)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (Character->IsPlayerControlled())
		{
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, MakeOutgoingGameplayEffectSpec(UGameplayEffect_CollideDamage::StaticClass()), UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OtherActor));
		}
	}
}

UGameplayEffect_CollideDamage::UGameplayEffect_CollideDamage()
{
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-10.f));
	ModifierInfo.TargetTags.IgnoreTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Ability.DamageImmune"));
	Modifiers.Add(ModifierInfo);
}

UGameplayAbility_DamageImmune::UGameplayAbility_DamageImmune()
{
	//ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	//NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
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
		if (UWorld* World = GetWorld())
		{
			// 태스크에 1000횟수를 넣어도, 어빌리티가 3초 뒤에 끝나기 때문에, 0.1초가 30번 하면 태스크도 끝남
			UAbilityTask_Repeat* RepeatTask = UAbilityTask_Repeat::RepeatAction(this, 0.1, 1000);
			RepeatTask->OnPerformAction.AddDynamic(this, &UGameplayAbility_DamageImmune::UpdateOpacityAndEmissive);
			RepeatTask->ReadyForActivation();

			Opacity = 0.3f;
			SetOpacity(Opacity);

			// GameplayEffect를 따로 만들지 않고 타이머로 구현
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
		Actor->ClearComponentOverlaps();
		SetMaterialParam([this](UMaterialInstanceDynamic* DynamicMaterialInstance) {
			DynamicMaterialInstance->SetScalarParameterValue("Opacity", 1.f);
			DynamicMaterialInstance->SetVectorParameterValue("Emissive", FVector4(0.f, 0.f, 0.f, 0.f));
		});
	}
}

void UGameplayAbility_DamageImmune::UpdateOpacityAndEmissive(int32 ActionNumber)
{
	if (UWorld* World = GetWorld())
	{
		Opacity = (FMath::Cos((2.f * PI) * (ActionNumber / 10.f)) + 3.f) * 2.f / 10.f;

		SetMaterialParam([this, ActionNumber](UMaterialInstanceDynamic* DynamicMaterialInstance) {
			DynamicMaterialInstance->SetScalarParameterValue("Opacity", Opacity);
			DynamicMaterialInstance->SetVectorParameterValue("Emissive", FVector4((30 - ActionNumber) / 30.f, 0.f, 0.f, 0.f));
		});
	}
}

void UGameplayAbility_DamageImmune::SetOpacity(float InOpacity)
{
	SetMaterialParam([InOpacity](UMaterialInstanceDynamic* DynamicMaterialInstance) {
		DynamicMaterialInstance->SetScalarParameterValue("Opacity", InOpacity);
	});
}

void UGameplayAbility_DamageImmune::SetMaterialParam(TFunction<void(UMaterialInstanceDynamic*)> Func)
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			for (int MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); ++MaterialIndex)
			{
				if (UMaterialInstanceDynamic* DynamicMaterialInstance = MeshComp->CreateDynamicMaterialInstance(MaterialIndex))
				{
					Func(DynamicMaterialInstance);
				}
			}
		}
	}
}