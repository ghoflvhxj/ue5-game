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
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UGeometryCollectionComponent* GeometryCollectionComponent = nullptr;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Destruct();
};
