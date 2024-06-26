// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "TestGame/MAttribute/MAttribute.h"

// 임시
#include "TestGame/MPlayerController/MPlayerController.h"

UGameplayAbility_MoveToMouse::UGameplayAbility_MoveToMouse()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Move")));

	if (IsValid(WaitTask))
	{
		WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_MoveToMouse::MoveToMouse);
		WaitTask->ReadyForActivation();
	}
}

void UGameplayAbility_MoveToMouse::MoveToMouse(FGameplayEventData Payload)
{
	APawn* Pawn = Cast<APawn>(Payload.Target);
	if (IsValid(Pawn) == false)
	{
		return;
	}
	
	if (AMPlayerControllerInGame* Controller = Cast<AMPlayerControllerInGame>(Pawn->GetController()))
	{
		if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this))
		{
			FHitResult HitResult;
			Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

			FVector TargetLocation = Pawn->GetActorLocation();
			TargetLocation = HitResult.bBlockingHit ? HitResult.Location : FVector::ZeroVector;

			if (NavigationSystem->NeedsLoadForClient())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, TargetLocation);
			}
			else
			{
				Controller->Server_PawnMoveToLocation(TargetLocation);
			}
		}
	}
}

UGameplayAbility_CharacterAction::UGameplayAbility_CharacterAction()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	//InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.BasicAttack"));
	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_CharacterAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsValid(WaitTask) == false && TriggerEventData != nullptr)
	{
		WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TriggerEventData->EventTag);
		WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_CharacterAction::Action);
		WaitTask->ReadyForActivation();
	}
}

void UGameplayAbility_CharacterAction::Action(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UGameplayAbility_BasicAttack::Action(FGameplayEventData Payload)
{
	Super::Action(Payload);
	StartBasicAttack(Payload);
}

void UGameplayAbility_BasicAttack::StartBasicAttack(const FGameplayEventData& Payload)
{
	if (IsValid(Payload.Instigator) == false)
	{
		return;
	}

	if (IsValid(PlayMontageTask) && PlayMontageTask->IsFinished() == false)
	{
		return;
	}

	float BasicAttackSpeed = 1.f;

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		bool bAttributeFound = false;
		BasicAttackSpeed = AbilitySystemComponent->GetGameplayAttributeValue(UMAttributeSet::GetBasicAttackSpeedAttribute(), bAttributeFound);
	}

	if (UMActionComponent* ActionComponent = Payload.Instigator->GetComponentByClass<UMActionComponent>())
	{
		PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName(TEXT("Attack")), ActionComponent->GetActionMontage(Payload.EventTag), BasicAttackSpeed);
		PlayMontageTask->ReadyForActivation();
	}
}

void UGameplayAbility_BasicAttack::CancelBasicAttack()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UGameplayAbility_BasicAttack::EndBasicAttack()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}