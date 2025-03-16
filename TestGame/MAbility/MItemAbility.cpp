// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MItemAbility.h"
#include "AbilitySystemComponent.h"
//#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
//#include "GameFramework/Pawn.h"
//#include "GameFramework/GameStateBase.h"
//#include "GameFramework/PlayerState.h"
//#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
//#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
//#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "NavigationSystem.h"

//#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
//#include "TestGame/MCharacter/MPlayer.h"
//#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/Bullet/Bullet.h"
//#include "TestGame/MAbility/MEffect.h"
//#include "TestGame/MItem/ItemBase.h"

DECLARE_LOG_CATEGORY_CLASS(LogItemAbility, Log, Log);

UGameplayAbility_Item::UGameplayAbility_Item()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::Type::ReplicateNo;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("AbilityType.Item"));
}

UGameplayAbility_DamageImmune::UGameplayAbility_DamageImmune()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Character.Damaged");
	AbilityTriggers.Add(TriggerData);
}

UGameplayAbility_AutoArrow::UGameplayAbility_AutoArrow()
{
}

void UGameplayAbility_AutoArrow::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (Character.IsValid() == false)
	{
		return;
	}

	if (HasAuthority(&CurrentActivationInfo) && Character->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, FTimerDelegate::CreateWeakLambda(this, [this]() {
			UWorld* World = GetWorld();
			AMCharacter* Character = GetCharacter();
			if (IsValid(Character) && IsValid(World))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = Character;
				SpawnParams.Instigator = Character;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				FRotator Rotation = GetCharacterRotation();

				FTransform SpawnTransform;
				SpawnTransform.SetLocation(GetCharacterLocation(true));
				SpawnTransform.SetScale3D(FVector::OneVector);

				int32 MaxArrowNum = GetParam(FGameplayTag::RequestGameplayTag("Ability.RadialArrow.Num"));
				for (int32 i = 0; i < MaxArrowNum; ++i)
				{
					FRotator SpawnRotation = Rotation;
					SpawnRotation.Yaw += i * (360.f / MaxArrowNum);

					SpawnTransform.SetRotation(SpawnRotation.Quaternion());
					
					
					if (ABullet* Bullet = World->SpawnActorDeferred<ABullet>(BulletClass, SpawnTransform, Character, Character, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
					{
						if (Bullet->GetClass()->ImplementsInterface(UActorByAbilityInterface::StaticClass()))
						{
							IActorByAbilityInterface::Execute_InitUsingAbility(Bullet, this);
						}
						InitAbilitySpawnedActor(Bullet);
						UGameplayStatics::FinishSpawningActor(Bullet, SpawnTransform);
						Bullet->StartProjectile();
					}
				}
			}
		}), 1.f, true);
	}
}

UGameplayAbility_Turret::UGameplayAbility_Turret()
{

}

void UGameplayAbility_Turret::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
		SpawnTurrets();
	});
}

void UGameplayAbility_Turret::SpawnTurrets()
{
	UWorld* World = GetWorld();
	if ((Character.IsValid() && IsValid(World)) == false)
	{
		return;
	}

	if ((HasAuthority(&CurrentActivationInfo) && Character->HasAuthority()) == false)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character.Get();
	SpawnParams.Instigator = Character.Get();

	FTransform SpawnTransform;
	SpawnTransform.SetScale3D(FVector::OneVector);

	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(this))
	{
		while (TurretCount < FMath::RoundToInt(GetParam(FGameplayTag::RequestGameplayTag("Ability.Turret.Num"))))
		{
			FNavLocation ResultLocation;
			if (NavSystem->GetRandomReachablePointInRadius(Character->GetActorLocation(), 200.f, ResultLocation, Cast<ANavigationData>(NavSystem->GetMainNavData()), NavSystem->CreateDefaultQueryFilterCopy()))
			{
				SpawnTransform.SetLocation(ResultLocation.Location);
			}

			if (AActor* Turret = World->SpawnActor<AActor>(TurretClass.TryLoadClass<AActor>(), SpawnTransform, SpawnParams))
			{
				if (Turret->GetClass()->ImplementsInterface(UActorByAbilityInterface::StaticClass()))
				{
					IActorByAbilityInterface::Execute_InitUsingAbility(Turret, this);
				}

				Turret->SetLifeSpan(20.f);
				Turret->OnEndPlay.AddDynamic(this, &UGameplayAbility_Turret::DecreaseTurretCount);
				++TurretCount;
			}
		}
	}
}

void UGameplayAbility_Turret::DecreaseTurretCount(AActor* InActor, EEndPlayReason::Type EndReason)
{
	--TurretCount;

	if (TurretCount == 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, FTimerDelegate::CreateWeakLambda(this, [this]() {
			SpawnTurrets();
		}), SpawnDelay, false);
	}
}

UGameplayAbility_DamageToOne::UGameplayAbility_DamageToOne()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.DamageToOne"));
}

void UGameplayAbility_DamageToOne::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

