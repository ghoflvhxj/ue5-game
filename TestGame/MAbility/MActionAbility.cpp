#pragma once

#include "MActionAbility.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_PlayAnimAndWait.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "CollisionDebugDrawingPublic.h"

#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "TestGame/MCharacter/MMonster.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MComponents/DamageComponent.h"
#include "SkillSubsystem.h"
#include "MGameInstance.h"

DECLARE_LOG_CATEGORY_CLASS(LogSkill, Log, Log);

UGameplayAbility_Skill::UGameplayAbility_Skill()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Skill"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Dead")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Move"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability"));
}

void UGameplayAbility_Skill::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGameplayAbility_Skill, SkillIndex, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UGameplayAbility_Skill, SkillIndex, COND_InitialOnly);
}

void UGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// BP라면 Super::ActivateAbility애서 커밋어빌리티가 호출됨.
	if (bHasBlueprintActivate == false)
	{
		if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	OnActive();

	if (bEndInstantly)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

bool UGameplayAbility_Skill::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		if (AMMonster* Monster = Cast<AMMonster>(Character))
		{
			const FActionTableRow ActionTableRow = Monster->GetActionTableRow();
			SkillIndex = ActionTableRow.SkillIndex;
		}
				
		return true;
	}

	return false;
}

void UGameplayAbility_Skill::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	//SkillSequenceIndex = INDEX_NONE;

	if (AMPlayer* PlayerCharacter = Cast<AMPlayer>(Character))
	{
		PlayerCharacter->FinishSkill();
		PlayerCharacter->MoveMode();
	}

	RemoveIndicator();	

	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : ActiveEffectHandles)
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveEffectHandle);
	}
}

void UGameplayAbility_Skill::OnLoad_Implementation()
{
	if (AMPlayer* PlayerCharacter = Cast<AMPlayer>(Character))
	{
		PlayerCharacter->SetLoadedSkillIndex(SkillIndex);
	}

	ExecuteSoundCue(FGameplayTag::RequestGameplayTag("GameplayCue.Sound.SkillLoad"));
}

void UGameplayAbility_Skill::OnActive_Implementation()
{
	ExecuteSoundCue(FGameplayTag::RequestGameplayTag("GameplayCue.Sound.SkillExecute"));

	// 버프
	for (const FBuffInfo& BuffInfo : GetSkillTableRow().BuffInfos)
	{
		if (BuffInfo.Target != EIGameplayEffectTarget::Player)
		{
			continue;
		}

		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(BuffInfo.EffectClass);
		if (EffectSpecHandle.IsValid() == false)
		{
			continue;
		}

		for (const TPair<FGameplayTag, float>& BuffParams : BuffInfo.TagToValue)
		{
			FGameplayTag BuffParamTag = BuffParams.Key;
			if (BuffInfo.TagToValue.Contains(BuffParamTag) == false)
			{
				UE_LOG(LogSkill, Error, TEXT("SkillID[%d], %s can't find %s In BuffInfo"), SkillIndex, *GetName(), *(BuffParamTag.ToString()));
				continue;
			}

			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, BuffParamTag, BuffInfo.TagToValue[BuffParamTag]);
			ActiveEffectHandles.Add(ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle));
		}
	}

	float Duration = GetParamUsingName("SkillParam.Duration");
	if (Duration > 0.f)
	{
		// Duration 이펙트
		if (IsValid(DurationEffectClass))
		{
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DurationEffectClass);
			EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FGameplayTag::RequestGameplayTag("Effect.Duration"), Duration);
			EffectSpecHandle.Data->AddDynamicAssetTag(DurationTag);
			DurationEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle);
		}

		GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, FTimerDelegate::CreateWeakLambda(Character.Get(), [this]() {
			UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
			if (IsValid(AbilitySystemComponent) == false)
			{
				return;
			}

			// Duration이 끝난 후에 실제 쿨이 돌도록 함
			if (HasAuthority(&CurrentActivationInfo))
			{
				AbilitySystemComponent->RemoveGameplayCue(DurationTag);

				UE_LOG(LogTemp, Warning, TEXT("OnActive Remove Infinity Cool"));
				//AbilitySystemComponent->RemoveActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(AbilityTags.Last().GetSingleTagContainer()));

				if (IsValid(SubCoolDownEffectClass))
				{
					FGameplayEffectSpecHandle CoolDownEffectSpecHandle = MakeOutgoingGameplayEffectSpec(SubCoolDownEffectClass);
					ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CoolDownEffectSpecHandle);
				}

				FGameplayCueParameters CueParams;
				CueParams.SourceObject = this;
				CueParams.Instigator = Character;
				CueParams.OriginalTag = SkillTag;
				CueParams.AggregatedSourceTags.AddTag(SkillTag);
				AbilitySystemComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.UI.SkillCool"), CueParams);

				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}
		}), Duration, false);
	}

	RemoveIndicator();
}

void UGameplayAbility_Skill::ExecuteSoundCue(FGameplayTag InTag)
{
	FGameplayCueParameters CueParams;
	CueParams.SourceObject = this;
	CueParams.Instigator = Character;
	CueParams.OriginalTag = SkillTag;
	CueParams.AggregatedSourceTags.AddTag(SkillTag);
	CueParams.Location = Character->GetActorLocation();

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(InTag, CueParams);
}

void UGameplayAbility_Skill::SetSkillIndex(int32 InIndex, FGameplayTag InSkillTag)
{
	if (SkillIndex != INDEX_NONE)
	{
		UE_LOG(LogSkill, Warning, TEXT("Attempt to change skillIndex from [%d] to [%d]."), SkillIndex, InIndex);
		return;
	}

	SkillIndex = InIndex;
	SkillTag = InSkillTag;
	const FSkillTableRow& SkillTableRow = UMGameInstance::GetSkillTableRow(this, SkillIndex);

	// 파라미터 설정
	AddParams(SkillTableRow.InitialAttributes);

	// 버프 설정
	for (const FBuffInfo& BuffInfo : SkillTableRow.BuffInfos)
	{
		if (IsValid(BuffInfo.EffectClass) == false)
		{
			continue;
		}

		MapEffectToParams.FindOrAdd(BuffInfo.EffectClass).Target = BuffInfo.Target;
		MapEffectToParams.FindOrAdd(BuffInfo.EffectClass).MapParamToValue = BuffInfo.TagToValue;
	}
	
	// 강화
	if (AMPlayer* PlayerCharacter = Cast<AMPlayer>(Character))
	{
		PlayerCharacter->GetSkillEnhancedDelegate().AddUObject(this, &UGameplayAbility_Skill::SkillEnhance);
	}
}

float UGameplayAbility_Skill::GetSkillParam(FGameplayTag GameplayTag)
{
	const FSkillTableRow& SkillTableRow = GetSkillTableRow();
	if (SkillTableRow.Params.Contains(GameplayTag))
	{ 
		return SkillTableRow.Params[GameplayTag];
	}

	return 0.f;
}

void UGameplayAbility_Skill::SkillEnhance(int32 SkillEnhanceIndex)
{
	UMGameInstance* GameInstance = GetWorld()->GetGameInstance<UMGameInstance>();
	if (IsValid(GameInstance) == false)
	{
		return;
	}

	const FSkillEnhanceTableRow& SkillEnhanceTableRow = GameInstance->GetSkillEnhanceTableRow(SkillEnhanceIndex);
	if (AbilityTags.HasAny(SkillEnhanceTableRow.AbilityTags) == false)
	{
		return;
	}

	AddParams(SkillEnhanceTableRow.Data);

	for (const FBuffInfo& EffectParam : SkillEnhanceTableRow.EffectParams)
	{
		for (const auto& ParamToValuePair : EffectParam.TagToValue)
		{
			MapEffectToParams.FindOrAdd(EffectParam.EffectClass).MapParamToValue.FindOrAdd(ParamToValuePair.Key) += ParamToValuePair.Value;
		}
	}
}

void UGameplayAbility_Skill::ApplyEffectToTarget(AActor* InEffectCauser, AActor* InTarget, const FGameplayAbilityTargetDataHandle& InTargetDataHandle)
{
	Super::ApplyEffectToTarget(InEffectCauser, InTarget, InTargetDataHandle);

	// 추가 GE
	for (const auto& EffectToParams : MapEffectToParams)
	{
		if (EffectToParams.Value.Target != EIGameplayEffectTarget::Monster) // Monster 이름을 Target으로 변경해야 할듯. 오브젝트 타입을 설정은 별도로 만들어야 할 거 같다.
		{
			continue;
		}

		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(EffectToParams.Key);
		FGameplayEffectParam EffectParams = EffectToParams.Value;
		for (const auto& ParamToValuePair : EffectParams.MapParamToValue)
		{
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(ParamToValuePair.Key, ParamToValuePair.Value);
		}
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, InTargetDataHandle);
	}
}

void UGameplayAbility_Skill::SpawnIndicator()
{
	const FSkillTableRow& SkillTableRow = UMGameInstance::GetSkillTableRow(this, SkillIndex);

	if (Character->IsLocallyControlled())
	{
		// 인디게이터는 내 화면에만 보이면 됨
		if (UClass* IndicatorClass = TSoftClassPtr<AActor>(SkillTableRow.Indicator).LoadSynchronous())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = Character.Get();
			SpawnParams.Owner = Character.Get();
			if (AActor* NewIndicator = GetWorld()->SpawnActor<AActor>(IndicatorClass, SpawnParams))
			{
				if (NewIndicator->GetClass()->ImplementsInterface(UActorByAbilityInterface::StaticClass()))
				{
					IActorByAbilityInterface::Execute_InitUsingAbility(NewIndicator, this);
				}

				FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, false);
				NewIndicator->AttachToActor(Character.Get(), AttachRules);
				Indicator = NewIndicator;
			}
		}
	}
}

void UGameplayAbility_Skill::RemoveIndicator()
{
	if (Indicator.IsValid())
	{
		Indicator->SetActorHiddenInGame(true);
		Indicator->SetLifeSpan(0.1f);
		Indicator = nullptr;
	}
}

const FSkillTableRow& UGameplayAbility_Skill::GetSkillTableRow()
{
	return UMGameInstance::GetSkillTableRow(this, SkillIndex);
}

UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Dash"));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Attack"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Action.Reload"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("ActionType.Dynamic"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability"));

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

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

	auto DashProcess = [this]() {
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->MaxWalkSpeed = 3000.f;
			MovementComponent->MaxAcceleration = MovementComponent->MaxWalkSpeed * 10.f;
			MovementComponent->GroundFriction = 0.f;
			UE_LOG(LogTemp, Warning, TEXT("DoDash"));
		}

		StartLocation = Character->GetActorLocation();

		if (RepeatTask = UAbilityTask_Repeat::RepeatAction(this, 0.001f, 200))
		{
			// 다른 물체와 부딪혔을때 어떻게 할지?
			//Character->OnActorBeginOverlap.AddDynamic(this, &UGameplayAbility_Dash::StopDash);

			RepeatTask->OnPerformAction.AddDynamic(this, &UGameplayAbility_Dash::AddMovementInput);
			RepeatTask->OnFinished.AddDynamic(this, &UGameplayAbility_Dash::ClearDash);
			RepeatTask->ReadyForActivation();

		}
	};

	if (UMActionComponent* ActionComponent = Character->GetComponentByClass<UMActionComponent>())
	{
		if (UAnimMontage* Montage = ActionComponent->GetActionMontage(AbilityTags.GetByIndex(1))) // 0은 Action.Skill이라 1로 Action.Dash 태그 얻어옴
		{
			if (Montage->HasRootMotion() == false)
			{
				DashProcess();
			}

			if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "Dash", Montage, 1.f, NAME_None, true, 1.f, 0.f))
			{
				PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Dash::OnAnimFinished);
				PlayMontageTask->ReadyForActivation();
				return;
			}
		}
		else if (UAnimSequence* Sequence = ActionComponent->GetActionSequence(AbilityTags.GetByIndex(1)))
		{
			if (UAbilityTask_PlayAnimAndWait* PlaySequenceTask = UAbilityTask_PlayAnimAndWait::CreatePlayAnimAndWaitProxy(this, "Dash", Sequence, TEXT("DefaultSlot"), 0.1f, 0.1f, 1.f, 0.f, true, 1.f))
			{
				PlaySequenceTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Dash::OnAnimFinished);
				PlaySequenceTask->ReadyForActivation();
				DashProcess();
				return;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGameplayAbility_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	ClearDash(INDEX_NONE);
}

void UGameplayAbility_Dash::AddMovementInput(int32 ActionIndex)
{
	if (Character.IsValid())
	{
		//if (StartLocation.Distance(Character->GetActorLocation()) > DashLength)
		//{
		//	ClearDash(0);
		//	return;
		//}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->AddInputVector(Character->GetActorForwardVector());
		}
	}
}

void UGameplayAbility_Dash::ClearDash(int32 ActionIndex)
{
	if (Character.IsValid() && RepeatTask != nullptr)
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			// Attribute로 가져오기
			MovementComponent->MaxWalkSpeed = 600.f;
			MovementComponent->GroundFriction = 8.f;
			MovementComponent->MaxAcceleration = 2048;
			MovementComponent->StopMovementImmediately();

			UE_LOG(LogTemp, Warning, TEXT("ClearDash"));
		}
	}

	RepeatTask = nullptr;
}

void UGameplayAbility_Dash::StopDash(AActor* OverlappedActor, AActor* OtherActor)
{
	//
}

void UGameplayAbility_Dash::OnAnimFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGameplayAbility_BattoSkill::OnActive_Implementation()
{
	Super::OnActive_Implementation();

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);

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

	if (AMPlayerControllerInGame* PlayerController = Character->GetController<AMPlayerControllerInGame>())
	{
		FVector Distance = { GetParamUsingName("Ability.Batto.Distance"), 0.f, 0.f};
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
				//DrawCapsuleOverlap(World, Location, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), Rotation.Quaternion(), OverlapResults, 5.f);
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

						ApplyEffect(GetAvatarActorFromActorInfo(), OverlapActor);
					}
				}

				//if (UAbilitySystemComponent* AbilityComponent = Character->GetComponentByClass<UAbilitySystemComponent>())
				//{
				//	FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
				//	FGameplayCueParameters CueParams(EffectContext);
				//	CueParams.Location = (Character->GetActorLocation() + GoalLocation) / 2.f;
				//	AbilityComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Batto"), CueParams);
				//}

				Character->SetActorLocation(GoalLocation);
			});

			NavSystem->FindPathAsync(NavSystem->GetDefaultSupportedAgent().DefaultProperties, PathFindingQuery, PathQueryDelegate);
		}
	}
}

bool UGameplayAbility_BattoSkill::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/)
{
	if (Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		UWorld* World = GetWorld();
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);

		return IsValid(World) && IsValid(NavSystem);
	}

	return false;
}


UGameplayAbility_SwordWave::UGameplayAbility_SwordWave()
{
}

void UGameplayAbility_SwordWave::OnActive_Implementation()
{
	Super::OnActive_Implementation();

	WeaponChangeDelegateHandle = Character->OnWeaponChangedEvent.AddWeakLambda(this, [this](AActor* Old, AActor* New) {
		if (AWeapon* OldWeapon = Cast<AWeapon>(Old))
		{
			OldWeapon->OnComboChangedEvent.Remove(WeaponComboChangeDelegateHandle);
			WeaponComboChangeDelegateHandle.Reset();
		}

		if (AWeapon* NewWeapon = Cast<AWeapon>(New))
		{
			BindToWeaponCombo();
		}
	});

	BindToWeaponCombo();
}

void UGameplayAbility_SwordWave::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (Character.IsValid())
	{
		Character->OnWeaponChangedEvent.Remove(WeaponChangeDelegateHandle);
	}

	if (BoundWeapon.IsValid())
	{
		BoundWeapon->OnComboChangedEvent.Remove(WeaponComboChangeDelegateHandle);
		WeaponComboChangeDelegateHandle.Reset();
	}
}

void UGameplayAbility_SwordWave::SpawnSwordWave()
{
	if (IsValid(BulletClass) == false)
	{
		UE_LOG(LogAttack, Warning, TEXT("Failed to spawn bullet. Class is empty."));
		return;
	}

	if (Character.IsValid() == false || BoundWeapon.IsValid() == false)
	{
		UE_LOG(LogAttack, Warning, TEXT("Failed to spawn bullet. Cahce data is invalid."));
		return;
	}

	if (Character->HasAuthority() == false)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = BoundWeapon.Get();
		SpawnParams.Instigator = Character.Get();

		FRotator SpawnRotation = GetCharacterRotation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetCharacterLocation(true));
		SpawnTransform.SetRotation(SpawnRotation.Quaternion());

		if (ABullet* Bullet = World->SpawnActor<ABullet>(BulletClass, SpawnTransform, SpawnParams))
		{
			if (Bullet->GetClass()->ImplementsInterface(UActorByAbilityInterface::StaticClass()))
			{
				IActorByAbilityInterface::Execute_InitUsingAbility(Bullet, this);
			}
			InitAbilitySpawnedActor(Bullet);
			Bullet->StartProjectile();
		}
	}
}

void UGameplayAbility_SwordWave::BindToWeaponCombo()
{
	if (Character.IsValid())
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			BoundWeapon = Weapon;
			WeaponComboChangeDelegateHandle = BoundWeapon->OnComboChangedEvent.AddWeakLambda(this, [this](int32 InCombo) {
				if (InCombo != INDEX_NONE)
				{
					SpawnSwordWave();
				}
			});
		}
	}
}