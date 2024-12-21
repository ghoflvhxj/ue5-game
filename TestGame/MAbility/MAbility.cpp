// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MAbility.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "NavigationSystem.h"

#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MItem/ItemBase.h"

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

void UGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	for (const FBuffInfo& BuffInfo : GetSkillInfo()->BuffInfos)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BuffInfo.EffectClass);
		if (SpecHandle.IsValid() == false)
		{
			continue;
		}

		for (const TPair<FGameplayTag, float>& BuffParams : BuffInfo.TagToValue)
		{
			FGameplayTag BuffParamTag = BuffParams.Key;
			if (BuffInfo.TagToValue.Contains(BuffParamTag) == false)
			{
				UE_LOG(LogAbility, Error, TEXT("SkillID[%d], %s Can't not find %s In BuffInfo"), SkillIndex, *GetName(), *(BuffParamTag.ToString()));
				continue;
			}

			SpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, BuffParamTag, BuffInfo.TagToValue[BuffParamTag]);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}

	if (bEndInstantly)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

bool UGameplayAbility_Skill::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		if (FSkillTableRow* SkillInfo = GetSkillInfo())
		{
			return true;
		}
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

	AActor* AbilityOwer = GetAvatarActorFromActorInfo();
	AbilityOwer->OnActorBeginOverlap.AddDynamic(this, &UGameplayAbility_CollideDamage::OnCollide);
	CueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Hit.Default");
	if (AWeapon* Weapon = Cast<AWeapon>(AbilityOwer))
	{
		if (const FGameItemTableRow* ItemTableRow = Weapon->GetItemTableRow())
		{
			// 테스트용
			switch (ItemTableRow->GameItemInfo.Grade)
			{
			case EItemGrade::Normal:
				CueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Test1");
				break;
			case EItemGrade::Rare:
				CueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Test2");
				break;
			case EItemGrade::Unique:
				CueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Test3");
				break;
			}
		}
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
		if (AMCharacter* OtherCharacter = Cast<AMCharacter>(OtherActor))
		{
			if (OtherCharacter->IsDead() == false && OtherCharacter->IsPlayerControlled() != MyCharacter->IsPlayerControlled())
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_Damage::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Attribute.Damage"), -Damage);
 
				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OtherActor));

				if (UAbilitySystemComponent* OtherAbilityComponent = OtherCharacter->GetComponentByClass<UAbilitySystemComponent>())
				{
					FGameplayEffectContextHandle EffectContext = OtherAbilityComponent->MakeEffectContext();
					FGameplayCueParameters CueParams(EffectContext);
					OtherAbilityComponent->ExecuteGameplayCue(CueTag, CueParams);
				}
			}
		}
	}
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

bool UGameplayAbility_WeaponBase::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		if (AMCharacter* NewCharacter = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
		{
			// 여기서 캐싱하여 ActivateAbility 호출할 때는 반드시 유효성이 보장이 되도록
			Character = NewCharacter;
			Weapon = NewCharacter->GetEquipItem<AWeapon>();
			return Character.IsValid() && Weapon.IsValid();
		}
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

	ClearCachedData();
}

void UGameplayAbility_WeaponBase::ClearCachedData()
{
	Weapon = nullptr;
	Character = nullptr;
}

UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
}

void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		// 로컬 입력이 있는 경우에만...
		if (Character->IsLocallyControlled())
		{
			FVector InputVector = FVector::ZeroVector;
			if (UInputCacheComponent* InputCacheComponent = Character->GetComponentByClass<UInputCacheComponent>())
			{
				InputVector = InputCacheComponent->GetInput();
			}
			else if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				InputVector = MovementComponent->GetLastInputVector();
			}

			// 대시 방향이 이상한 경우가 있던데, Unreliable RPC라 가끔씩 씹히는 경우로 확인됨. 어떻게 해결할지는 이후에 생각함
			if (bool bForward = InputVector.X != 0.f)
			{
				Character->SetActorRotation(FRotator(0.0, InputVector.X >= 0.0 ? 0.0 : 180.0, 0.0));
				Character->Server_SetTargetAngle(InputVector.X >= 0.0 ? 0.0 : 180.0, true);
			}
			if (bool bStrafe = InputVector.Y != 0.f)
			{
				Character->SetActorRotation(FRotator(0.0, InputVector.Y >= 0.0 ? 90.0 : 270.0, 0.0));
				Character->Server_SetTargetAngle(InputVector.Y >= 0.0 ? 90.0 : 270.0, true);
			}
		}

		if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
		{
			if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
			{
				UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Dash", Montage, 1.f, NAME_None, true, 1.f, 0.f);
				PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Dash::OnMontageFinished);
				PlayMontageTask->ReadyForActivation();
				return;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_Dash::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UGameplayAbility_SpinalReflex::UGameplayAbility_SpinalReflex()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Event.Damaged");
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
			if (NavSystem->GetRandomReachablePointInRadius(Actor->GetActorLocation(), 2000.f, Result))
			{
				Actor->SetActorLocation(Result.Location);
			}
		}
	}

	if (UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
		FGameplayCueParameters CueParams(EffectContext);
		AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.FloatMessage.Teleport"), CueParams);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UGameplayAbility_CounterAttack::UGameplayAbility_CounterAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Event.Damaged");
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