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

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Event.Move")));

	WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_MoveToMouse::MoveToMouse);
	WaitTask->Activate();
}

void UGameplayAbility_MoveToMouse::MoveToMouse(FGameplayEventData Payload)
{
	APawn* Pawn = Cast<APawn>(Payload.Target);
	if (IsValid(Pawn) == false)
	{
		return;
	}

	if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
	{
		FVector TargetLocation = FVector::ZeroVector;
		if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(this))
		{
			FHitResult HitResult;
			Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

			TargetLocation = HitResult.bBlockingHit ? HitResult.Location : FVector::ZeroVector;
		}

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, TargetLocation);
	}
}

void UGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_BasicAttack::Test);

	if (IsValid(WaitTask) == false)
	{
		WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TriggerEventData->EventTag);
		WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_CharacterAction::Action);
		WaitTask->ReadyForActivation();
	}
}

void UGameplayAbility_BasicAttack::BasicAttack(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj %s"), *FString(__FUNCTION__));

	if (IsValid(Payload.Instigator))
	{
		if (UMActionComponent* ActionComponent = Payload.Instigator->GetComponentByClass<UMActionComponent>())
		{
			PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName(TEXT("Attack")), ActionComponent->GetActionMontage(Payload.EventTag));
			PlayMontageTask->Activate();
		}
	}
}

void UGameplayAbility_BasicAttack::Test()
{
	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Cancel"));
}
