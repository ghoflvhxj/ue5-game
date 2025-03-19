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
	
protected:
	virtual void BeginPlay();
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState);

public:
	// PlayerState가 유효한지 모든 검사를 수행 후 HUD에 등록하는 함수
	void AddToHUD();
	void AddToHUDInternal();
protected:
	FTimerHandle HudTimerHandle;

public:
	void Die();
	UFUNCTION()
	void OnRep_Dead();
	FOnPlayerDeadEvent& GetPlayerDeadEvent() { return OnPlayerDeadEvent; }
	bool IsDead() const { return bDead; }
protected:
	FOnPlayerDeadEvent OnPlayerDeadEvent;
	UPROPERTY(ReplicatedUsing=OnRep_Dead)
	bool bDead = false;

public:
	void SetCharacterIndex(int32 InIndex) { CharacterIndex = InIndex; }
	int32 GetCharacterIndex() const { return CharacterIndex; }
protected:
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly)
	int32 CharacterIndex = INDEX_NONE;
};
