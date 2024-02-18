// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.generated.h"

UINTERFACE(BlueprintType)
class CLIENT_API UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class CLIENT_API IInteractInterface 
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interaction();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnTargeted(AActor* Interactor);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnUnTargeted(AActor* Interactor);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsInteractable(AActor* Interactor);
};