// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "ItemBase.h"

#include "TestGame/Interface/InteractInterface.h"

#include "DropItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetedDelegate, bool, bTargeted);

USTRUCT(BlueprintType)
struct FDropItemInfo
{
	GENERATED_BODY()
};

UCLASS()
class TESTGAME_API ADropItem : public AItemBase, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADropItem();

	// 컴포넌트
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

// 아이템 정보
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDropItemInfo DropItemInfo;

// 상호작용
public:
	virtual void OnTargeted_Implementation(AActor* Interactor);
	virtual void OnUnTargeted_Implementation(AActor* Interactor);
	virtual bool IsInteractable_Implementation(AActor* Interactor);
protected:
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FOnInteractTargetedDelegate OnInteractTargetedDelegate;
};
