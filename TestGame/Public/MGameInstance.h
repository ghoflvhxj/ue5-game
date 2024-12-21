// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "MGameInstance.generated.h"

struct FGameItemTableRow;

UCLASS(Blueprintable)
class TESTGAME_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

public:
	FGameItemTableRow* GetItemTableRow(FName InRowName);
	FGameItemTableRow* GetItemTableRow(int32 InItemIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable = nullptr;
};
