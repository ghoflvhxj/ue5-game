// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "TestGame/MAttribute/MAttribute.h"

// 임시
#include "TestGame/MPlayerController/MPlayerController.h"

DECLARE_LOG_CATEGORY_CLASS(LogAbility, Log, Log);

void UMAbilityDataAsset::GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, TMap<FGameplayTag, FGameplayAbilitySpecHandle>& Handles) const
{ 
	checkf(IsValid(AbilitySystemComponent), TEXT("ASC Invalid."));
	AActor* Actor = AbilitySystemComponent->GetOwner<AActor>();
	checkf(IsValid(Actor), TEXT("ASC Invalid."));

	for (const FMAbilityBindInfo& BindInfo : Abilities)
	{
		if (IsValid(BindInfo.GameplayAbilityClass) == false)
		{
			UE_LOG(LogAbility, Warning, TEXT("Trying give Invalid AbilityClass to %s"), *(Actor->GetActorNameOrLabel()));
			continue;
		}

		FGameplayTag GameplayTag = BindInfo.GameplayTag;

		Handles.Emplace(GameplayTag, AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass)));
		if (BindInfo.bActivate && Handles.Contains(GameplayTag))
		{
			if (AbilitySystemComponent->TryActivateAbility(Handles[GameplayTag], true))
			{
				UE_LOG(LogAbility, Warning, TEXT("Initial TryActivateAbility Success %s"), *BindInfo.GameplayAbilityClass->GetName())
			}
			else
			{
				UE_LOG(LogAbility, Warning, TEXT("Initial TryActivateAbility Failed"))
			}
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

	UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));

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
			Character->UpdateTargetAngle();
			Character->SetRotateToTargetAngle(true);
		}

		if (UMActionComponent* ActionComponent = TriggerEventData->Instigator->GetComponentByClass<UMActionComponent>())
		{
			if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
			{
				PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName(TEXT("Attack")), Montage, BasicAttackSpeed, NAME_None, false);
				PlayMontageTask->ReadyForActivation();

				FTimerHandle THandle;
				TriggerEventData->Instigator->GetWorldTimerManager().SetTimer(THandle, FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]() {
					EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
				}), IsValid(Montage) ? Montage->GetPlayLength() : 1.f, false);
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
		return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags)/* && Character->IsAttackable()*/;
	}

	return false;
}