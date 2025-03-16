// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Chest.generated.h"

class USceneComponent;
class USphereComponent;
class UGeometryCollectionComponent;

UCLASS()
class TESTGAME_API ADestructableActor : public AActor
{
	GENERATED_BODY()

public:
	ADestructableActor();
protected:
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//UBoxComponent* BoxComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* SceneComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UGeometryCollectionComponent* GeometryCollectionComponent = nullptr;

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
	FVector GetItemSpawnLocation();

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Destruct(AActor* Destructor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Destruct(AActor* Desturctor);
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bDestructed = false;

protected:
	UPROPERTY(EditDefaultsOnly)
	bool bUseDrop = false;
	UPROPERTY(EditDefaultsOnly)
	int32 DropIndex = INDEX_NONE;
};
