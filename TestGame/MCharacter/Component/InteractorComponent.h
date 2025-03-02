// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "TestGame/Interface/InteractInterface.h"

#include "InteractorComponent.generated.h"

UENUM(BlueprintType)
enum class EInteractChannel : uint8
{
	None,
	Unique,
	Multiple,
};

UENUM(BlueprintType)
enum class EInteractState : uint8
{
	Wait,
	Doing,
};

USTRUCT(BlueprintType)
struct FInteractData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInteractChannel Channel;
};

DECLARE_EVENT(UMInteractorComponent, FInteractEvent);
DECLARE_EVENT_OneParam(UMInteractorComponent, FInteractFinishEvent, bool);

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class TESTGAME_API UMInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMInteractorComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool IsInteractable() { return IsValid(InteratingActor) == false; }
public:
	void Indicate(AActor* InActor);
	bool Interact(AActor* InActor);
	void SuccessInteract();
	void CancelInteract();
	void FinishInteract(bool bSuccess);
	bool IsChannel(EInteractChannel InChannel) { return InteractData.Channel == InChannel; }
public:
	FInteractEvent InteractStartEvent;
	FInteractFinishEvent InteractFinishEvent;
protected:
	FInteractData InteractData;
	FTimerHandle InteractTimerHandle;

public:
	void AddInteractor(AActor* InActor);
	UFUNCTION(Server, Reliable)
	void Server_ChangeInteractor(AActor* InActor);

public:
	template <class T>
	T* GetInteractor()
	{
		return Cast<T>(InteratingActor);
	}
	bool IsInteractingActor(AActor* InActor);
	UFUNCTION()
	void OnRep_Interactor();
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Interactor)
	AActor* InteratingActor = nullptr;
	EInteractState InteractState = EInteractState::Wait;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bFinishInstantly = false;
};
