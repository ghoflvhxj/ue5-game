// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

#include "Bullet.generated.h"

class UAbilitySystemComponent;
class UProjectileMovementComponent;
class UGameplayEffect;
class UGameplayEffect_Damage;
class UMDamageComponent;

UINTERFACE(BlueprintType)
class TESTGAME_API UBulletShooterInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IBulletShooterInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void InitBullet(ABullet* InBullet);
};

UCLASS()
class TESTGAME_API ADamageGiveActor : public AActor
{
	GENERATED_BODY()

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void React(AActor* InActor);
protected:
	bool IsReactable(AActor* InActor);
protected:
	TArray<TWeakObjectPtr<AActor>> IgnoreActors;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bDamagable = true;
};

UCLASS()
class TESTGAME_API ABullet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABullet();

protected:
	virtual void BeginPlay() override;
public:
	void OnBulletHit(AActor* InHitCauser, AActor* InActor);

public:
	void DestroyBullet();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMDamageComponent* DamageComponent = nullptr;

public:
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintCallable)
	void StartProjectile(const FVector& NewDirection, float NewDamage);
	void StartProjectile();
protected:
	FVector Direction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bPenerate = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bExplosion = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionRadius = 0.f;

protected:
	FDelegateHandle BulletHitHandleDelegate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UFXSystemAsset* FXAsset = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ExplosionSound = nullptr;

	// GC로 사운드 재생할 여건이 안되는 경우 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* HitSound = nullptr;
};

