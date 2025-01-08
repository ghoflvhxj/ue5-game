// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"

#include "MBattleComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTGAME_API UMTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMTeamComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void SetTeamIndex(int32 InTeamIndex);
public:
	UFUNCTION(BlueprintCallable)
	bool IsSameTeam(AActor* OtherActor) const;
	bool IsSameTeam(UMTeamComponent* OtherBattleComponent) const;
	bool IsSameTeam(int CheckTeamIndex) const;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	int TeamIndex = INDEX_NONE;
};
