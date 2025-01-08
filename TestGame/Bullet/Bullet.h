// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"

#include "Bullet.generated.h"

class UAbilitySystemComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

UCLASS()
class TESTGAME_API ADamageGiveActor : public AActor
{
	GENERATED_BODY()

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	bool IsReactable(AActor* InActor);
protected:
	TArray<TWeakObjectPtr<AActor>> IgnoreActors;
};

UCLASS()
class TESTGAME_API ABullet : public ADamageGiveActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABullet();

protected:
	virtual void BeginPlay() override;
public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

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
	//UAbilitySystemComponent* AbilitySystemComponent;

public:
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintCallable)
	void StartProjectile(const FVector& NewDirection, float NewDamage);
protected:
	FVector Direction;
	float Damage = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bPenerate = false;
};

