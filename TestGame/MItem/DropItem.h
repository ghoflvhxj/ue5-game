// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "ItemBase.h"

#include "TestGame/Interface/InteractInterface.h"

#include "DropItem.generated.h"

class UMInteractorComponent;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetedDelegate, bool, bTargeted);

UCLASS()
class TESTGAME_API ADropItem : public AItemBase
{
	GENERATED_BODY()

public:
	ADropItem();

	// 컴포넌트
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMInteractorComponent* InteractorComponent;

public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
protected:
	virtual void BeginPlay() override;

public:
	virtual void OnRep_ItemIndex();

protected:
	void UpdateItemMesh();

public:	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWeapon> WeaponClass = nullptr;

public:
	//virtual void Interaction_Implementation(AActor* Interactor);
	//virtual void OnTargeted_Implementation(AActor* Interactor);
	//virtual void OnUnTargeted_Implementation(AActor* Interactor);
	//virtual bool IsInteractable_Implementation(AActor* Interactor);
protected:
	//UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	//FOnInteractTargetedDelegate OnInteractTargetedDelegate;
};
