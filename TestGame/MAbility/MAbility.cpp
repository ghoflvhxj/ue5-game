// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MPlayerController/MPlayerController.h"

void UGameplayAbility_MoveToMouse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AMCharacter* MCharacter = Cast<AMCharacter>(ActorInfo->OwnerActor);
	if (IsValid(MCharacter) == false)
	{
		return;
	}

	AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(MCharacter->GetController());
	if (IsValid(PlayerController))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(PlayerController, PlayerController->GetMouseWorldPosition());
	}


	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj UGameplayAbility_MoveToMouse::ActivateAbility"));
}
