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

	if (AActor* AbilityOwner = GetAvatarActorFromActorInfo())
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Character.Move.Block"));
		UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(AbilityOwner, TagContainer);

		FTimerHandle DummyHandle;
		AbilityOwner->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, AbilityOwner, TagContainer]() {
			UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(AbilityOwner, TagContainer);
		}));

		if (UMActionComponent* ActionComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UMActionComponent>())
		{
			if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
			{
				PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "BasicAttack", Montage, BasicAttackSpeed, NAME_None, true, 1.f, 0.f);
				PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_BasicAttack::OnMontageFinished);

				PlayMontageTask->ReadyForActivation();
				return;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
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
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			Weapon->FinishBasicAttack();
		}
	}

	UE_LOG(LogAbility, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

bool UGameplayAbility_BasicAttack::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */)
{
	if (AMCharacter* Character = Cast<AMCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
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

	if (IsValid(MyCharacter))
	{
		if (AMCharacter* OtherCharacter = Cast<AMCharacter>(OtherActor))
		{
			if (OtherCharacter->IsDead() == false && OtherCharacter->IsPlayerControlled() != MyCharacter->IsPlayerControlled())
			{
				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, MakeOutgoingGameplayEffectSpec(UGameplayEffect_CollideDamage::StaticClass()), UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OtherActor));
			}
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

UGameplayAbility_KnockBack::UGameplayAbility_KnockBack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
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
			MovementComponent->AddRadialImpulse(Owner->GetActorLocation(), Radius, Strength, ERadialImpulseFalloff::RIF_Linear, false);
			//MovementComponent->AddImpulse(FVector(1000000.0, 0.0, 0.0));
		}
	}
}

UGameplayAbility_CameraShake::UGameplayAbility_CameraShake()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
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
			Weapon->IncreaseCombo();
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
			return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
		}
	}

	return false;
}

UGameplayAbility_BasicAttackStop::UGameplayAbility_BasicAttackStop()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
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

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
