// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "ItemBase.h"

#include "TestGame/Interface/InteractInterface.h"

#include "DropItem.generated.h"

class UMInteractorComponent;
class UNiagaraComponent;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetedDelegate, bool, bTargeted);

UCLASS()
class TESTGAME_API ADropItem : public AItemBase
{
	GENERATED_BODY()

public:
	ADropItem();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* NiagaraComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMInteractorComponent* InteractorComponent;

	UPrimitiveComponent* ActualPrimitiveComponent = nullptr;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay() override;

public:
	TArray<UAbilitySystemComponent*> GetItemEffectTargets(const FGameplayEffectParam& InEffectParam);

public:
	virtual void OnRep_ItemIndex() override;
protected:
	UFUNCTION(BlueprintNativeEvent, CallInEditor)
	void UpdateItemMesh();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath DefaultMeshPath;

protected:
	bool bGoToInteractor = false;
	float FollowingElapsedTime = 0.f;
protected:
	UPROPERTY(EditDefaultsOnly)
	bool bTest = false;
};