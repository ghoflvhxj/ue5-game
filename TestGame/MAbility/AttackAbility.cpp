#pragma once

#include "AttackAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"
#include "CollisionDebugDrawingPublic.h"

#include "TestGame/TestGame.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MPlayerController/MPlayerController.h"

DECLARE_LOG_CATEGORY_CLASS(LogAttack, Log, Log);

UGameplayAbility_AttackBase::UGameplayAbility_AttackBase()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.Attack.Block"));
}

bool UGameplayAbility_AttackBase::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return Weapon->IsAttackable();
	}

	return false;
}

void UGameplayAbility_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ComboDelegateHandle.IsValid() == false)
	{
		ComboDelegateHandle = Weapon->OnComboChangedEvent.AddUObject(this, &UGameplayAbility_AttackBase::MontageJumpToComboSection);
	}

	Weapon->SetAttackMode(AbilityTags.GetByIndex(0));
}

void UGameplayAbility_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsActive() == false)
	{
		return;
	}

	if (Weapon.IsValid())
	{
		Weapon->ResetCombo();
		Weapon->SetAttackMode(FGameplayTag::EmptyTag);
		if (ComboDelegateHandle.IsValid())
		{
			Weapon->OnComboChangedEvent.Remove(ComboDelegateHandle);
		}
		ComboDelegateHandle.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGameplayAbility_AttackBase::PlayAttackMontage()
{
	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0)))
		{
			PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Attack", Montage, GetAttackSpeed(), NAME_None, false, 1.f, 0.f, true);
			PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_AttackBase::OnMontageFinished);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_AttackBase::OnMontageFinished);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_AttackBase::OnMontageFinished);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_AttackBase::OnMontageFinished);

			UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj PlayAttackMontage"));

			PlayMontageTask->ReadyForActivation();
			return true;
		}
	}

	return false;
}

void UGameplayAbility_AttackBase::MontageJumpToComboSection(int32 InComboIndex)
{
	if (Character.IsValid() == false)
	{
		return;
	}

	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (IsValid(AnimInstance) == false)
	{
		return;
	}

	UAnimMontage* Montage = nullptr;
	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(0));
	}
	
	if (IsValid(Montage) == false)
	{
		return;
	}
	
	FString ComboName = InComboIndex == INDEX_NONE ? TEXT("End") : FString::Printf(TEXT("Combo%d"), InComboIndex);
	if (Montage->IsValidSectionName(*ComboName) == false)
	{
		ComboName = Montage->GetSectionName(Montage->GetNumSections() - 1).ToString();
	}

	UE_NLOG(LogAttack, Warning, TEXT("Combo MontageJumpToComboSection : % s"), *ComboName);

	float SectionStart = 0.f;
	float SectionEnd = 0.f;
	Montage->GetSectionStartAndEndTime(Montage->GetSectionIndex(*ComboName), SectionStart, SectionEnd);
	FAnimNotifyContext NotifyContext;
	Montage->GetAnimNotifiesFromDeltaPositions(SectionStart, SectionEnd, NotifyContext);

	for (const FAnimNotifyEventReference& NotifyRef : NotifyContext.ActiveNotifies)
	{
		const FAnimNotifyEvent* Notify = NotifyRef.GetNotify();
		if (Notify == nullptr)
		{
			continue;
		}

		if (Notify->NotifyName != TEXT("Combo"))
		{
			continue;
		}

		float NotifyTriggerTime = ((Notify->GetTriggerTime() - SectionStart) / GetAttackSpeed()) - (HasAuthority(&CurrentActivationInfo) ? 0.1f : 0.f);
		Character->GetWorldTimerManager().SetTimer(WeaponFinishCoolDownHandle, FTimerDelegate::CreateWeakLambda(Character.Get(), [this]() {
			if (Weapon.IsValid())
			{
				Weapon->FinishCoolDown();
			}
		}), NotifyTriggerTime, false);

		UE_NLOG(LogAttack, Warning, TEXT("Combo Timer:%f"), NotifyTriggerTime);

		break;
	}

	if (AnimInstance->Montage_IsPlaying(Montage) && AnimInstance->Montage_GetCurrentSection(Montage) != ComboName)
	{
		AnimInstance->Montage_JumpToSection(*ComboName, Montage);
	}
}

void UGameplayAbility_AttackBase::OnMontageFinished()
{
	UE_LOG(LogAttack, Warning, TEXT("OnMontageFinished"));

	if (Weapon.IsValid())
	{
		Weapon->FinishCoolDown();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

float UGameplayAbility_AttackBase::GetAttackSpeed() const
{
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetAttackSpeedAttribute());
	}

	return AttackSpeed;
}

UGameplayAbility_BasicAttack::UGameplayAbility_BasicAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(GetLightAttackTag());
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.DashLight"));		// 공격 어빌리티가 시작되면 다른 공격 어빌리티는 모두 취소 되도록 해야함
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.Light.Block"));	// 약공격만 블락할 경우
}

void UGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//if (const FWeaponData* WeaponData = Weapon->GetWeaponData())
	//{
	//	// 임시작업. 무기마다 다른 이펙트가 들어갈 수 있음.
	//	if (WeaponData->WeaponType == EWeaponType::Gun)
	//	{
	//		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_AddMoveSpeed::StaticClass());
	//		EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GetEffectValueTag(), -300.f);
	//		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
	//	}
	//}

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsActive() == false)
	{
		return;
	}

	if (Weapon.IsValid())
	{
		if (const FWeaponData* WeaponData = Weapon->GetWeaponData())
		{
			if (WeaponData->WeaponType == EWeaponType::Gun)
			{
				FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_AddMoveSpeed::StaticClass());
				EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GetEffectValueTag(), 300.f);
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

int32 UGameplayAbility_BasicAttack::GetEffectIndex() const
{
	if (Weapon.IsValid())
	{
		return Weapon->GetEffectIndex();
	}

	return INDEX_NONE;
}

UGameplayAbility_DashLightAttack::UGameplayAbility_DashLightAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.DashLight"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash")); // Character.Dash
}

void UGameplayAbility_DashLightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

UGameplayAbility_LightChargeAttack::UGameplayAbility_LightChargeAttack()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.ChargeLight"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(GetLightAttackTag());
}

void UGameplayAbility_LightChargeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (PlayAttackMontage())
	{
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_LightChargeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsActive() == false)
	{
		return;
	}

	if (Character.IsValid())
	{
		Character->FinishCharge();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UGameplayAbility_Batto::UGameplayAbility_Batto()
{
//	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
//	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
//	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
//
//	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack.ChargeLight"));
//	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
//	CancelAbilitiesWithTag.AddTag(LightAttack);

	//bClearCacheIfEnd = false;
}

void UGameplayAbility_Batto::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//if (CommitAbility(Handle, ActorInfo, ActivationInfo, nullptr) == false)
	//{
	//	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//	return;
	//}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//if (PlayAttackMontage())
	//{
	//	return;
	//}

	//EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameplayAbility_Batto::MontageJumpToComboSection(int32 InComboIndex)
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);

	if (Character.IsValid() == false || IsValid(World) == false || IsValid(NavSystem) == false)
	{
		return;
	}

	Super::MontageJumpToComboSection(InComboIndex);

	FVector Trace = { 0.f, 0.f, 10000.f };

	FVector LineTraceStart = Character->GetActorLocation() + Trace;
	FVector LineTraceEnd = Character->GetActorLocation() - Trace;
	FCollisionObjectQueryParams ObjectQueryParams(ECC_WorldStatic);
	FCollisionQueryParams QueryParams;
	QueryParams.TraceTag = TEXT("Batto");
	QueryParams.AddIgnoredActor(Character.Get());

#if WITH_EDITOR
	QueryParams.bDebugQuery = true;
#endif

	if (GetWorld())
	{
		//GetWorld()->DebugDrawSceneQueries(QueryParams.TraceTag);
		//GetWorld()->DebugDrawTraceTag = QueryParams.TraceTag;
	}

	FString ComboName = InComboIndex == INDEX_NONE ? TEXT("End") : FString::Printf(TEXT("Combo%d"), InComboIndex);
	if (ComboName == TEXT("Combo0"))
	{
		if (AMPlayerControllerInGame* PlayerController = Character->GetController<AMPlayerControllerInGame>())
		{
			FVector Distance = { 1000.f, 0.f, 0.f };
			Distance = Distance.RotateAngleAxis(PlayerController->GetYawToMouse(), FVector::UpVector);

			LineTraceStart += Distance;
			LineTraceEnd += Distance;
		}

		if (ANavigationData* NavData = Cast<ANavigationData>(NavSystem->GetMainNavData()))
		{
			FHitResult HitResult;
			if (World->LineTraceSingleByObjectType(HitResult, LineTraceStart, LineTraceEnd, ObjectQueryParams, QueryParams))
			{
				FPathFindingQuery PathFindingQuery;
				PathFindingQuery.Owner = Character;
				PathFindingQuery.StartLocation = Character->GetActorLocation();
				PathFindingQuery.EndLocation = HitResult.Location;
				PathFindingQuery.bAllowPartialPaths = true;
				PathFindingQuery.NavData = NavData;
				PathFindingQuery.NavAgentProperties = NavSystem->GetDefaultSupportedAgent().DefaultProperties;
				PathFindingQuery.bRequireNavigableEndLocation = false;
				PathFindingQuery.QueryFilter = NavData->GetDefaultQueryFilter();

				FNavPathQueryDelegate PathQueryDelegate = FNavPathQueryDelegate::CreateWeakLambda(this, [this, World, QueryParams](uint32, ENavigationQueryResult::Type QueryResult, FNavPathSharedPtr Path) {
					if (Character.IsValid() == false || QueryResult != ENavigationQueryResult::Success)
					{
						return;
					}

					FVector GoalLocation = Path->GetPathPoints().Last().Location;
					FRotator RotateToGoal = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), GoalLocation);

					TArray<FOverlapResult> OverlapResults;
					FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(100.f, static_cast<float>((GoalLocation - Character->GetActorLocation()).Length() / 2.f));

					FVector Location = Character->GetActorLocation() + (GoalLocation - Character->GetActorLocation()) / 2.f;
					FRotator Rotation = { 90.f + RotateToGoal.Pitch, RotateToGoal.Yaw, RotateToGoal.Roll };
#if WITH_EDITOR
					DrawCapsuleOverlap(World, Location, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), Rotation.Quaternion(), OverlapResults, 5.f);
#endif
					if (World->OverlapMultiByObjectType(OverlapResults, Location, Rotation.Quaternion(), FCollisionObjectQueryParams(ECC_Pawn), CapsuleShape, QueryParams))
					{
						TSet<AActor*> DamagedActor;
						for (const FOverlapResult& OverlapResult : OverlapResults)
						{
							AActor* OverlapActor = OverlapResult.GetActor();
							if (DamagedActor.Contains(OverlapActor) == false)
							{
								DamagedActor.Add(OverlapActor);
							}
							else
							{
								continue;
							}

							if (AMCharacter* OverlapCharacter = Cast<AMCharacter>(OverlapActor))
							{
								FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UGameplayEffect_Damage::StaticClass());
								ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OverlapCharacter));
							}
						}
					}

					if (UAbilitySystemComponent* AbilityComponent = Character->GetComponentByClass<UAbilitySystemComponent>())
					{
						FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
						FGameplayCueParameters CueParams(EffectContext);
						CueParams.Location = (Character->GetActorLocation() + GoalLocation) / 2.f;
						AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Batto"), CueParams);
					}

					Character->SetActorLocation(GoalLocation);
					});

				NavSystem->FindPathAsync(NavSystem->GetDefaultSupportedAgent().DefaultProperties, PathFindingQuery, PathQueryDelegate);
			}
		}
	}
}
