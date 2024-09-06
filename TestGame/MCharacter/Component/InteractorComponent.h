// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "TestGame/Interface/InteractInterface.h"

#include "InteractorComponent.generated.h"

USTRUCT(BlueprintType)
struct FInteractData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractingTime = 1.f;
};

DECLARE_EVENT(UMInteractorComponent, FInteractEvent);

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class TESTGAME_API UMInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMInteractorComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void Indicate(AActor* InActor);
	bool Interact(AActor* InActor);
	void CancelInteract();
public:
	FInteractEvent InteractStartEvent;
	FInteractEvent InteractFinishEvent;
protected:
	FInteractData InteractData;
	FTimerHandle InteractTimerHandle;

public:
	void ChangeInteractor(AActor* InActor);
	UFUNCTION(Server, Reliable)
	void Server_ChangeInteractor(AActor* InActor);

public:
	template <class T>
	T* GetInteractor()
	{
		return Cast<T>(InteratingActor);
	}
	bool IsInteractingActor(AActor* InActor);
protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	AActor* InteratingActor = nullptr;
};
