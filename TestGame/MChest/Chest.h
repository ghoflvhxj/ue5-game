// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Chest.generated.h"

class UGeometryCollectionComponent;

UCLASS()
class TESTGAME_API ADestructableActor : public AActor
{
	GENERATED_BODY()

public:
	ADestructableActor();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* BoxComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UGeometryCollectionComponent* GeometryCollectionComponent = nullptr;

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Destruct(AActor* Desturctor);
};
