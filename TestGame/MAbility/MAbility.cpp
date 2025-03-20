// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "NavigationSystem.h"
#include "Engine/AssetManager.h"

#include "CharacterLevelSubSystem.h"
#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MComponents/DamageComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MItem/ItemBase.h"
#include "TestGame/Bullet/Bullet.h"
#include "TestGame/MFunctionLibrary/MContainerFunctionLibrary.h"

DECLARE_LOG_CATEGORY_CLASS(LogAbility, Log, Log);

void UMAbilityDataAsset::PostLoad()
{
	Super::PostLoad();

	for (const FMAbilityBindInfo& AbilityBindInfo : Abilities)
	{
		AbilityMap.FindOrAdd(AbilityBindInfo.GameplayTag) = AbilityBindInfo;
	}
}

void UMAbilityDataAsset::GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent) const
{ 
	FGameplayTagContainer FilterTagContainer;
	for (const FMAbilityBindInfo& AbilityBindInfo : Abilities)
	{
		FilterTagContainer.AddTag(AbilityBindInfo.GameplayTag);
	}

	GiveAbilities(AbilitySystemComponent, FilterTagContainer);
}

void UMAbilityDataAsset::GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Filter) const
{
	if (IsValid(AbilitySystemComponent) == false)
	{
		UE_LOG(LogAbility, Warning, TEXT("Attempt to use invalid abilitycomponent."));
		return;
	}

	if (AbilitySystemComponent->IsNetSimulating())
	{
		return;
	}

	for (const FMAbilityBindInfo& BindInfo : Abilities)
	{
		FGameplayTag AbilityTag = BindInfo.GameplayTag;

		if (Filter.HasTag(AbilityTag) == false)
		{
			continue;
		}

		if (IsValid(BindInfo.GameplayAbilityClass) == false)
		{
			continue;
		}

		if (FGameplayAbilitySpec* FoundAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(BindInfo.GameplayAbilityClass))
		{
			++FoundAbilitySpec->Level;
			AbilitySystemComponent->MarkAbilitySpecDirty(*FoundAbilitySpec);
		}
		else
		{
			FGameplayAbilitySpecHandle NewAbilitySpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass, 1, BindInfo.InputID));
			if (BindInfo.bActivate)
			{
				AbilitySystemComponent->TryActivateAbility(NewAbilitySpecHandle, true);
			}
		}
	}
}

void UMAbilityDataAsset::ClearAbilities(UAbilitySystemComponent* AbilitySystemComponent) const
{
	for (const FMAbilityBindInfo& BindInfo : Abilities)
	{
		if (IsValid(BindInfo.GameplayAbilityClass) == false)
		{
			continue;
		}

		if (FGameplayAbilitySpec* FoundAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(BindInfo.GameplayAbilityClass))
		{
			AbilitySystemComponent->ClearAbility(FoundAbilitySpec->Handle);
		}
	}
}

void UMAbilityDataAsset::ConverToMap(TMap<FGameplayTag, TSubclassOf<UGameplayAbility>>& TagToAbilityClassMap) const
{
	for (const FMAbilityBindInfo& AbilityBindInfo : Abilities)
	{
		TagToAbilityClassMap.FindOrAdd(AbilityBindInfo.GameplayTag) = AbilityBindInfo.GameplayAbilityClass;
	}
}

FMAbilityBindInfo UMAbilityDataAsset::GetBindInfo(FGameplayTag InTag)
{
	if (AbilityMap.Contains(InTag))
	{
		return AbilityMap[InTag];
	}

	return FMAbilityBindInfo();
}

bool UMAbilityDataAsset::HasAbilityTag(FGameplayTag InTag) const
{
	for (const FMAbilityBindInfo& AbilityBindInfo : Abilities)
	{
		if (AbilityBindInfo.GameplayTag == InTag)
		{
			return true;
		}
	}

	return false;
}

UGameplayAbility_MoveToMouse::UGameplayAbility_MoveToMouse()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.Move")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Dead")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.Attack.Light")));
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
	
	if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
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

UGameplayAbility_CollideDamage::UGameplayAbility_CollideDamage()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateNo;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
}

void UGameplayAbility_CollideDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UMDamageComponent* DamageComponent = Character->GetComponentByClass<UMDamageComponent>())
	{
		DamageComponent->GetOnDamageEvent().AddUObject(this, &UGameplayAbility_CollideDamage::OnCollide);
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

	ApplyBuffByDamageEvent(MyCharacter, OtherActor);
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
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Damaged");
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

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Move"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Move")); // Trigger로 동작해서 적용되지 않는듯?

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

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->AddMovementInput(FVector(Foward, Strafe, 0.f));
		Character->SetMoving(true);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameplayAbility_Move::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		//UE_CLOG(HasAuthority(&CurrentActivationInfo), LogTemp, Warning, TEXT("ghoflvhxj SetMoving False"));
		Character->SetMoving(false);
	}
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

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.Combo"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); 
		return; 
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Weapon.Get());
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_ConsumeAmmo::StaticClass());
		EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.ConsumeMagazine"), -1);
		ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, EffectSpecHandle, TargetDataHandle);
	}
	
	Weapon->NextCombo();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGameplayAbility_Combo::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		if (Weapon->IsAttackable())
		{
			return true;
		}
		else if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->TryActivateAbilityByClass(UGameplayAbility_BasicAttackStop::StaticClass(), true);
			return false;
		}
	}

	return false;
}

void UGameplayAbility_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UGameplayAbility::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (Character.IsValid())
	{
		if (Character->OnWeaponChangedEvent.IsBoundToObject(this) && Character->OnWeaponChangedEvent.Remove(WeaponChangedDelegateHandle))
		{
			WeaponChangedDelegateHandle.Reset();
		}
	}
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
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 총 사격이 Loop하는 몽타주라 손을 때면 End섹션으로 이동시키는 역할
	// 그런데 이렇게 하는게 최선일까?
	if (const FWeaponData* WeaponData = Weapon->GetWeaponData())
	{
		if (WeaponData->WeaponType == EWeaponType::Gun)
		{
			Weapon->ResetCombo();
		}
	}

	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.Attack.Light")))
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
		if (UAnimMontage* AttackMontage = ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.Attack.Light")))
		{
			if (AttackMontage == Montage)
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}
		}

		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UGameplayAbility_BasicAttackStop::OnMontageEnd);
	}
}

UGameplayAbility_Reload::UGameplayAbility_Reload()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	float ReloadSpeed = 1.f;

	if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
	{
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Reload", Montage, ReloadSpeed, NAME_None, true, 1.f, 0.f);
			PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Reload::OnMontageFinished);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Reload::OnMontageFinished);

			PlayMontageTask->ReadyForActivation();
			return;
		}
	}
}

void UGameplayAbility_Reload::OnMontageFinished()
{
	if (Weapon.IsValid())
	{
		UE_CLOG(GetWorld()->IsNetMode(NM_DedicatedServer), LogTemp, Warning, TEXT("Server Reload"));
		UE_CLOG(GetWorld()->IsNetMode(NM_Client), LogTemp, Warning, TEXT("Client Reload"));
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Weapon.Get());
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_Reload::StaticClass());
		EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.Ammo"), 30.f);
		EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.TotalAmmo"), -30.f);
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UGameplayAbility_CharacterBase::UGameplayAbility_CharacterBase()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameplayAbility_CharacterBase, SerializedParamsMap);
}

void UGameplayAbility_CharacterBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo());
}

bool UGameplayAbility_CharacterBase::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		// 캐릭터 유효성이 반드시 보장되도록 함
		return Character.IsValid();
	}

	return false;
}

FGameplayEffectSpecHandle UGameplayAbility_CharacterBase::MakeOutgoingGameplayEffectSpecWithIndex(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 InEffectIndex, float Level /*= 1.f*/) const
{
	FGameplayEffectSpecHandle NewEffectSpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, GameplayEffectClass, Level);

	if (NewEffectSpecHandle.IsValid())
	{
		FGameplayEffectContext* EffectContext = NewEffectSpecHandle.Data->GetContext().Get();
		if (EffectContext != nullptr && EffectContext->GetScriptStruct() == FMGameplayEffectContext::StaticStruct())
		{
            FMGameplayEffectContext* MEffectContext = static_cast<FMGameplayEffectContext*>(EffectContext);
			MEffectContext->EffectIndex = InEffectIndex;
		}
	}

	return NewEffectSpecHandle;
}

class UMAbilitySystemComponent* UGameplayAbility_CharacterBase::GetMAbilitySystem()
{
	return Cast<UMAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

AMCharacter* UGameplayAbility_CharacterBase::GetCharacter()
{
	return Character.Get();
}

FVector UGameplayAbility_CharacterBase::GetCharacterLocation(bool bIncludeCapsuleHeight)
{
	FVector OutLocation = FVector::ZeroVector;
	if (Character.IsValid())
	{
		OutLocation = Character->GetActorLocation();
		if (bIncludeCapsuleHeight)
		{
			if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
			{
				OutLocation.Z += CapsuleComponent->GetScaledCapsuleHalfHeight();
			}
		}
	}
	else
	{
		UE_LOG(LogAbility, Warning, TEXT("%s failed. Character is invalid."), *FString(__FUNCTION__));
	}

	return OutLocation;
}

FVector UGameplayAbility_CharacterBase::GetCapsuleHalfHeight()
{
	FVector Out = FVector::ZeroVector;
	if (Character.IsValid())
	{
		if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			Out.Z += CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
	}

	return Out;
}

FRotator UGameplayAbility_CharacterBase::GetCharacterRotation()
{
	FRotator OutRotator = FRotator::ZeroRotator;

	if (Character.IsValid())
	{
		OutRotator.Yaw = Character->GetActorRotation().Yaw;
	}

	return OutRotator;
}

void UGameplayAbility_CharacterBase::InitAbilitySpawnedActor(AActor* InActor)
{
	if (IsValid(InActor) == false)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (InActor->IsA<ABullet>())
		{
			bool bProjectileScaling = false;
			float ProjectileScale = AbilitySystemComponent->GetGameplayAttributeValue(UMAttributeSet::GetProjectileScaleAttribute(), bProjectileScaling);
			if (bProjectileScaling)
			{
				InActor->SetActorScale3D(FVector(ProjectileScale));
			}
		}
	}

	if (InActor->GetClass()->ImplementsInterface(UActorByAbilityInterface::StaticClass()))
	{
		IActorByAbilityInterface::Execute_InitUsingAbility(InActor, this);
	}

	if (UMDamageComponent* DamageComponent = InActor->GetComponentByClass<UMDamageComponent>())
	{
		DamageComponent->GetOnDamageEvent().AddUObject(this, &UGameplayAbility_CharacterBase::ApplyBuffByDamageEvent);
	}
}

void UGameplayAbility_CharacterBase::ApplyBuffByNoneEvent()
{

}

void UGameplayAbility_CharacterBase::ApplyBuffByDamageEvent(AActor* InEffectCauser, AActor* InTarget)
{
	if (IsValid(InEffectCauser) == false)
	{
		return;
	}

	if (IsValid(InTarget) == false)
	{
		return;
	}

	if (IsValid(DamageEffectClass) == false)
	{
		return;
	}

	FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(InTarget);
	UpdateDynamicParams(InTarget);

	ApplyEffectToTarget(DamageEffectClass, InEffectCauser, InTarget, TargetDataHandle);
}

void UGameplayAbility_CharacterBase::ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, AActor* InCauser, AActor* InTarget, const FGameplayAbilityTargetDataHandle& InTargetDataHandle)
{
	UMAbilitySystemComponent* TargetASC = InTarget->GetComponentByClass<UMAbilitySystemComponent>();

	if (IsValid(TargetASC) == false)
	{
		return;
	}

	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpecWithIndex(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectClass, GetEffectIndex());
	
	if (EffectClass->IsChildOf(UGameplayEffect_Damage::StaticClass()))
	{
		// Damage 파라미터 추출
		for (const auto& ParamToValuePair : MapParamToValue)
		{
			if (ParamToValuePair.Key.MatchesTag(FGameplayTag::RequestGameplayTag("DamageParam")) == false)
			{
				continue;
			}

			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, ParamToValuePair.Key, ParamToValuePair.Value);
		}
		for (const auto& DynamicParamToValuePair : DynamicParamToValue)
		{
			if (DynamicParamToValuePair.Key.MatchesTag(FGameplayTag::RequestGameplayTag("DamageParam")) == false)
			{
				continue;
			}

			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, DynamicParamToValuePair.Key, DynamicParamToValuePair.Value);
		}

		EffectSpecHandle.Data.Get()->Period = GetParamUsingName("DamageParam.Period");
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, InTargetDataHandle);

		// HitGC 로직. CueParam을 커스텀으로 채우려면 직접 ExecutePlayCue를 호출해야 함 ex)Location
		FGameplayCueParameters CueParams;
		CueParams.EffectCauser = InCauser;
		CueParams.EffectContext = EffectSpecHandle.Data->GetContext();
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.Location = InCauser->GetActorLocation() + (InTarget->GetActorLocation() - InCauser->GetActorLocation()) / 2.f;

		FGameplayTag SourceTag = AbilityTags.Filter(FGameplayTag::RequestGameplayTag("Ability").GetSingleTagContainer()).Last();
		if (SourceTag == FGameplayTag::EmptyTag)
		{
			SourceTag = AbilityTags.Last();
		}
		CueParams.AggregatedSourceTags.AddTag(SourceTag);

		// GE가 어떤 Cue를 실행할지 저장하고 있어서 가져옴
		FGameplayTag DamageCueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Default");
		if (UGameplayEffect_Damage* GEDamageCDO = Cast<UGameplayEffect_Damage>(DamageEffectClass->GetDefaultObject()))
		{
			DamageCueTag = GEDamageCDO->GetDamageCue();
			GEDamageCDO->UpdateCueParams(InCauser, InTarget, CueParams);
		}

		TargetASC->ExecuteGameplayCue(DamageCueTag, CueParams);
	}
	else
	{
		UE_LOG(LogAbility, Error, TEXT("Gameplayeffect[%s] has no handle logic."), *EffectClass->GetName());
	}
}

void UGameplayAbility_CharacterBase::SetEnableCharacterOverlapDamage(bool bInEnable)
{
	if (Character.IsValid() == false)
	{
		return;
	}

	UMDamageComponent* DamageComponent = Character->GetComponentByClass<UMDamageComponent>();
	if (IsValid(DamageComponent) == false)
	{
		return;
	}

	if (bInEnable)
	{
		CharacterOverlapDamageDelegateHandle = DamageComponent->GetOnDamageEvent().AddUObject(this, &UGameplayAbility_CharacterBase::ApplyBuffByDamageEvent);
		DamageComponent->Activate(true);
	}
	else if(CharacterOverlapDamageDelegateHandle.IsValid())
	{
		DamageComponent->GetOnDamageEvent().Remove(CharacterOverlapDamageDelegateHandle);
		CharacterOverlapDamageDelegateHandle.Reset();
		DamageComponent->Reset();
	}
}

void UGameplayAbility_CharacterBase::UpdateDynamicParams_Implementation(AActor* OtherActor)
{

}

void UGameplayAbility_CharacterBase::AddParam(const FGameplayTag& InTag, float InValue)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	MapParamToValue.FindOrAdd(InTag) += InValue;

	UContainerFunctionLibrary::SerializeMap(MapParamToValue, SerializedParamsMap);
}

void UGameplayAbility_CharacterBase::AddParams(const TMap<FGameplayTag, float>& InParams)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	for (const TPair<FGameplayTag, float>& InitialAttribute : InParams)
	{
		MapParamToValue.FindOrAdd(InitialAttribute.Key) += InitialAttribute.Value;
	}

	UContainerFunctionLibrary::SerializeMap(MapParamToValue, SerializedParamsMap);
}

void UGameplayAbility_CharacterBase::SetParams(const TMap<FGameplayTag, float>& InParams)
{
	MapParamToValue.Empty();

	AddParams(InParams);
}

float UGameplayAbility_CharacterBase::GetParam(FGameplayTag InTag)
{
	return MapParamToValue.FindOrAdd(InTag);
}

float UGameplayAbility_CharacterBase::GetParamUsingName(FName InName)
{
	return GetParam(FGameplayTag::RequestGameplayTag(InName));
}

void UGameplayAbility_CharacterBase::OnRep_Params()
{
	UContainerFunctionLibrary::DeserializeMap(MapParamToValue, SerializedParamsMap);
}

void UGameplayAbility_CharacterBase::SetDynamicParam(FGameplayTag InTag, float InValue)
{
	DynamicParamToValue.FindOrAdd(InTag) = InValue;
}

void UGameplayAbility_CharacterBase::SerializeAttributeMap(FArchive& Archive)
{
	if (Archive.IsSaving())
	{
		int32 Num = MapParamToValue.Num();
		Archive << Num;

		for (auto& Elem : MapParamToValue)
		{
			Archive << Elem.Key;
			Archive << Elem.Value;
		}
	}
	else if (Archive.IsLoading())
	{
		MapParamToValue.Empty();
		int32 Num;
		Archive << Num;

		for (int32 i = 0; i < Num; ++i)
		{
			FGameplayTag Key;
			float Value;
			Archive << Key;
			Archive << Value;
			MapParamToValue.FindOrAdd(Key) = Value;
		}
	}
}

void UGameplayAbility_WeaponBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (Character.IsValid())
	{
		Character->OnWeaponChangedEvent.AddUObject(this, &UGameplayAbility_WeaponBase::UpdateWeapon);
	}
}

bool UGameplayAbility_WeaponBase::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return Weapon.IsValid();
	}

	return false;
}

void UGameplayAbility_WeaponBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (Character->OnWeaponChangedEvent.IsBoundToObject(this) == false)
	{
		WeaponChangedDelegateHandle = Character->OnWeaponChangedEvent.AddWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo](AActor* Old, AActor* New) {
			if (Old == Weapon)
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			}
		});
	}
}

void UGameplayAbility_WeaponBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (Character.IsValid())
	{
		if (Character->OnWeaponChangedEvent.IsBoundToObject(this) && Character->OnWeaponChangedEvent.Remove(WeaponChangedDelegateHandle))
		{
			WeaponChangedDelegateHandle.Reset();
		}
	}
}

void UGameplayAbility_WeaponBase::UpdateWeapon(AActor* Old, AActor* New)
{
	if (WeaponDamageDelegateHandle.IsValid())
	{
		if (UMDamageComponent* DamageComponent = Old->GetComponentByClass<UMDamageComponent>())
		{
			DamageComponent->GetOnDamageEvent().Remove(WeaponDamageDelegateHandle);
		}
		WeaponDamageDelegateHandle.Reset();
	}

	Weapon = Cast<AWeapon>(New);
	if (Weapon.IsValid())
	{
		if (UMDamageComponent* DamageComponent = Weapon->GetComponentByClass<UMDamageComponent>())
		{
			WeaponDamageDelegateHandle = DamageComponent->GetOnDamageEvent().AddUObject(this, &UGameplayAbility_WeaponBase::ApplyBuffByDamageEvent);
		}
	}
}

UGameplayAbility_SpinalReflex::UGameplayAbility_SpinalReflex()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Damaged");
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));

	// 공격 취소 & 막음
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack"));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_SpinalReflex::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(this))
		{
			AActor* Actor = GetAvatarActorFromActorInfo();
			FNavLocation Result;
			if (NavSystem->GetRandomReachablePointInRadius(Actor->GetActorLocation(), 1000.f, Result))
			{
				Actor->SetActorLocation(Result.Location);
			}
		}
	}

	if (UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
		FGameplayCueParameters CueParams(EffectContext);
		AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.UI.Floater.Teleport"), CueParams);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UGameplayAbility_CounterAttack::UGameplayAbility_CounterAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Damaged");
	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
}

void UGameplayAbility_CounterAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AActor* Actor = GetAvatarActorFromActorInfo();
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = Cast<APawn>(Actor);
		SpawnParams.Owner = Actor;

		FVector Offset = FVector::ZeroVector;
		if (ACharacter* Character = Cast<ACharacter>(Actor))
		{
			if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
			{
				Offset.Z += CapsuleComponent->GetScaledCapsuleHalfHeight();
			}
		}

		if (HasAuthority(&ActivationInfo))
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Actor->GetActorLocation() + Offset);
			if (AActor* CounterAttacker = World->SpawnActor(CounterAttackClass, &SpawnTransform, SpawnParams))
			{

			}
		}

		if (UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
			FGameplayCueParameters CueParams(EffectContext);
			CueParams.Instigator = Actor;
			CueParams.Location = Actor->GetActorLocation() + Offset;
			AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.UI.Floater.Counter"), CueParams);
			AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Counter"), CueParams);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}


UGameplayAbility_Start::UGameplayAbility_Start()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FGameplayTag StartTag = FGameplayTag::RequestGameplayTag("Ability.Start");

	AbilityTags.AddTag(StartTag);
	//ActivationOwnedTags.AddTag(StartTag);
	ActivationBlockedTags.AddTag(StartTag);
}

void UGameplayAbility_Start::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetActorHiddenInGame(false);
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
		if (IsValid(AnimInstance))
		{
			if (UAnimMontage* StartMontage = GetStartAnim())
			{
				GetAbilitySystemComponentFromActorInfo()->PlayMontage(this, ActivationInfo, StartMontage, 1.f);
				AnimInstance->OnMontageEnded.AddDynamic(Character, &AMCharacter::OnStartAnimFinished);
			}
		}

		//if (UAnimMontage* StartMontage = GetStartAnim())
		//{
		//	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Start", StartMontage))
		//	{
		//		PlayMontageTask->ReadyForActivation();
		//	}
		//}
	}
	
	// 계속 StartTag를 가지고 있어야 하니 끝내지 않음
}

void UGameplayAbility_Start::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->StopMontageIfCurrent(*GetStartAnim());
	}
}

UAnimMontage* UGameplayAbility_Start::GetStartAnim()
{
	if (AActor* Actor = GetAvatarActorFromActorInfo())
	{
		if (UMActionComponent* ActionComponent = Actor->GetComponentByClass<UMActionComponent>())
		{
			return ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.Start"));
		}
	}

	return nullptr;
}

void UGameplayAbility_LevelUp::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	if (Character.IsValid())
	{
		if (ULevelComponent* LevelComponent = Character->GetComponentByClass<ULevelComponent>())
		{
			LevelComponent->GetOnLevelUpEvent().AddUObject(this, &UGameplayAbility_LevelUp::UpdateMesh);
		}
	}

	for (const auto& LevelToMeshPair : LevelToMesh)
	{
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamableManager.RequestAsyncLoad(LevelToMeshPair.Value, []() {});
	}
}

void UGameplayAbility_LevelUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_LevelUp::UpdateMesh(int32 InLevel)
{
	if (Character.IsValid() == false)
	{
		return;
	}

	FSoftObjectPath MeshPath;
	for (const auto& LevelToMeshPair : LevelToMesh)
	{
		if (LevelToMeshPair.Key <= InLevel)
		{
			MeshPath = LevelToMeshPair.Value;
		}
	}

	if (MeshPath.IsValid() == false)
	{
		return;
	}

	USkeletalMeshComponent* OldMesh = Character->GetMesh();
	UAnimMontage* OldMontage = Character->GetCurrentMontage();
	FName Section = NAME_None;
	float Pos = 0.f;
	float PlayRate = 1.f;
	if (IsValid(OldMesh) && IsValid(OldMontage))
	{
		Section = OldMesh->GetAnimInstance()->Montage_GetCurrentSection(OldMontage);
		float SectionStart = 0.f;
		float SectionEnd = 0.f;
		OldMontage->GetSectionStartAndEndTime(OldMontage->GetSectionIndex(Section), SectionStart, SectionEnd);
		Pos = OldMesh->GetAnimInstance()->Montage_GetPosition(OldMontage) - SectionStart;
		PlayRate = OldMesh->GetAnimInstance()->Montage_GetPlayRate(OldMontage);
	}

	Character->GetMesh()->SetSkeletalMesh(Cast<USkeletalMesh>(MeshPath.TryLoad()), false);

	if (IsValid(OldMontage))
	{
		GetAbilitySystemComponentFromActorInfo()->PlayMontage(this, CurrentActivationInfo, OldMontage, PlayRate, Section, Pos);
	}
}
