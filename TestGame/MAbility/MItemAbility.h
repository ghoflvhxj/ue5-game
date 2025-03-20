// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"

#include "MAbility.h"

#include "MItemAbility.generated.h"

class ABullet;

UCLASS()
class TESTGAME_API UGameplayAbility_Item : public UGameplayAbility_CharacterBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Item();
};

UCLASS()
class TESTGAME_API UGameplayAbility_DamageImmune : public UGameplayAbility_Item
{
	GENERATED_BODY()

public:
	UGameplayAbility_DamageImmune();
};

UCLASS()
class TESTGAME_API UGameplayAbility_AutoArrow : public UGameplayAbility_Item
{
	GENERATED_BODY()

public:
	UGameplayAbility_AutoArrow();

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABullet> BulletClass = nullptr;
	FTimerHandle SpawnTimer;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Turret : public UGameplayAbility_Item
{
	GENERATED_BODY()

public:
	UGameplayAbility_Turret();

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
public:
	void SpawnTurrets();
	UFUNCTION()
	void DecreaseTurretCount(AActor* InActor, EEndPlayReason::Type EndReason);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftClassPath TurretClass;
	int32 TurretCount = 0;

	FTimerHandle SpawnTimer;
	float SpawnDelay = 5.f;
};

UCLASS()
class TESTGAME_API UGameplayAbility_DamageToOne : public UGameplayAbility_Item
{
	GENERATED_BODY()

public:
	UGameplayAbility_DamageToOne();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
