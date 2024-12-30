// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

DECLARE_EVENT(AMPlayerState, FOnPlayerDeadEvent)

UCLASS()
class TESTGAME_API AMPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:
	void Die();
	UFUNCTION()
	void OnRep_Dead();
	FOnPlayerDeadEvent& GetPlayerDeadEvent() { return OnPlayerDeadEvent; }
protected:
	FOnPlayerDeadEvent OnPlayerDeadEvent;
	UPROPERTY(ReplicatedUsing=OnRep_Dead)
	bool bDead = false;
};
