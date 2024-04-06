// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MPlayerController/MPlayerController.h"

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Event.Move")));

	WaitTask->EventReceived.AddDynamic(this, &UGameplayAbility_MoveToMouse::MoveToMouse);
	WaitTask->Activate();

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj UGameplayAbility_MoveToMouse::ActivateAbility"));
}

void UGameplayAbility_MoveToMouse::MoveToMouse(FGameplayEventData Payload)
{
	AMCharacter* MCharacter = Cast<AMCharacter>(Payload.Target);
	if (IsValid(MCharacter) == false)
	{
		return;
	}

	AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(MCharacter->GetController());
	if (IsValid(PlayerController))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(PlayerController, PlayerController->GetMouseWorldPosition());
	}
}
