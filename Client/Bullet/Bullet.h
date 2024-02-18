// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"

#include "Bullet.generated.h"

class UAbilitySystemComponent;
class UProjectileMovementComponent;
class UMBattleComponent;
class UGameplayEffect;

UCLASS()
class CLIENT_API ABullet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABullet();

public:
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

// GameplayAbilitySystem;
public:
	UFUNCTION(BlueprintCallable)
	void GiveEffects(UAbilitySystemComponent* AbilitySystemComponent);
protected:
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC = nullptr;
	TArray<UGameplayEffect*> GameplayEffects;

public:
	void DestroyBullet();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//UMBattleComponent* BattleComponent;

public:
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintCallable)
	void StartProjectile(const FVector& NewDirection, float NewDamage);
protected:
	//bool bFired = false;
	bool bProjectileable = false;
	FVector Direction;

protected:
	float Damage = 0.f;
};

