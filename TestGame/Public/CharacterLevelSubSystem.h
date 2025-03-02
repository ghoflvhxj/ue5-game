// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CharacterLevelSubSystem.generated.h"

UCLASS()
class TESTGAME_API UCharacterLevelSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void AddExperiance(AActor* InActor, int32 Exp);
};

USTRUCT(BlueprintType)
struct FExperienceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentExperience = 0;
};

UCLASS()
class TESTGAME_API ULevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULevelComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintPure)
	const FExperienceInfo& GetCurrentExperience() { return CurrentExperience; }
	UFUNCTION(BlueprintPure) 
	virtual int32 GetNextExperiance() { return 10 + (CurrentExperience.Level - 1) * 2; }
	void AddExperiance(int32 InValue);
	UFUNCTION()
	void OnRep_CurrentExperience(const FExperienceInfo& OldExpInfo);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceChangedEvent, const FExperienceInfo&, InExpInfo);
	UPROPERTY(BlueprintAssignable)
	FOnExperienceChangedEvent OnExperienceChangedEvent;
	DECLARE_EVENT_OneParam(ULevelComponent, FOnLevelUpEvent, int32 InLevel);
	FOnLevelUpEvent& GetOnLevelUpEvent() { return OnLevelUp; }
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentExperience)
	FExperienceInfo CurrentExperience;
	FOnLevelUpEvent OnLevelUp;

	// 테스트용
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_LevlUp();
};