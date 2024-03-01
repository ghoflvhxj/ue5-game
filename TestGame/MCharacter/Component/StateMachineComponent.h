// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "StateMachineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChangedDynamicDelegate, uint8, OldValue, uint8, NewValue);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStateChangedDelegate, uint8 OldValue, uint8 NewValue);

#define STATECLASS(X) StaticEnum<X>()

UCLASS()
class TESTGAME_API UStateClass : public UObject
{
public:
	GENERATED_BODY()

public:
	UStateClass()
	{

	}

	UStateClass(UEnum* NewEnumClass)
		: EnumClass(NewEnumClass)
	{

	}

public:
	UPROPERTY(BlueprintReadOnly)
	UEnum* EnumClass;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTGAME_API UStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStateComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

// 상태 클래스 추가
public:
	UFUNCTION(BlueprintCallable)
	bool AddStateClass(TSubclassOf<UStateClass> StateClass);
	template <typename T>
	bool AddStateClass()
	{
		return AddStateClassInternal(STATECLASS(T));
	}
	bool AddStateClassInternal(UEnum* EnumClass);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<UEnum*, int> StateEnumClassToIndexMap;

// 상태 변경
public:
	UFUNCTION(BlueprintCallable)
	bool ChangeState(TSubclassOf<UStateClass> StateClass, uint8 NewStateValue);
	template <typename T>
	bool ChangeState(T NewStateValue)
	{
		return ChangeStateInternal(STATECLASS(T),(uint8)NewStateValue);
	}
private:
	bool ChangeStateInternal(UEnum* EnumClass, uint8 NewStateValue);

// 상태 얻기
public:
	UFUNCTION(BlueprintCallable)
	bool GetState(TSubclassOf<UStateClass> StateClass, uint8& OutStateValue);
public:
	template <typename T>
	bool GetState(T& OutStateValue)
	{
		return GetStateInternal(STATECLASS(T), (uint8&)OutStateValue);
	}
	template <typename T>
	T GetState()
	{
		T OutState = (T)0;
		GetStateInternal(STATECLASS(T), (uint8&)OutState);
		return OutState;
	}
private:
	bool GetStateInternal(UEnum* EnumClass, uint8& OutStateValue);

// 유틸
private:
	int GetStateIndex(UEnum* EnumClass);

public:
	void AddOnStateChangeDelegate(TSubclassOf<UStateClass> StateClass, UObject* Object, const TFunctionRef<void(uint8, uint8)> Func);
	TMap<UEnum*, FOnStateChangedDelegate> StateChangedDelegatesMap;
private:
	void AddOnStateChangeDelegateInternal(UEnum* StateEnumClass, UObject* Object, const TFunctionRef<void(uint8, uint8)> Func);
protected:
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnStateChangedDynamicDelegate OnStatChanged;

protected:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	TArray<uint8> States;
};
