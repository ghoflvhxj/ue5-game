// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"

#include "MBattleComponent.generated.h"

UENUM()
enum class EAttackType
{
	Target,
	Range,
	Count
};

UENUM()
enum class ETargetSearchType
{
	Circle,
	Square,
	Cone,
	Line,
	Count
};

UENUM()
enum class ERangeAttackType
{
	Circle,
	Square,
	Cone,
	Count
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTGAME_API UMBattleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMBattleComponent();
	
public:
	void Attack(TArray<AActor*> Targets);
	FTimerHandle AttackHandle;

public:
	bool IsAttackable();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAttackable;

public:
	UFUNCTION(BlueprintCallable)
	bool IsSameTeam(UMBattleComponent* OtherBattleComponent) const;
	bool IsSameTeam(int CheckTeamIndex) const;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int TeamIndex;

	// 스탯 컴포넌트로 이동...
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "체력"))
	int Hp;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "공격 타입"))
	EAttackType AttackType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "공격 범위"))
	float AttackRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "초당 몇번의 공격을 하는지"))
	float AttackCountPerSecond; 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "타겟 공격 타입시 몇명을 타겟팅 하는지"))
	int AttackTargetCount;

public:
	bool IsAttackableTarget(AActor* Target);
	bool SearchAttackableTargets();
	bool PickTargets(TArray<AActor*>& InTargets, TArray<AActor*>& OutPickedTargets);
protected:
	TArray<AActor*> AttackTagets;
	float AttackTick = 0.f;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class ABullet> BulletClass;
};
