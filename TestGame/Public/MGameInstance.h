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
	void IterateItemTable(TFunction<void(const FGameItemTableRow& GameItemTableRow)> Function);
	FGameItemTableRow* GetItemTableRow(FName InRowName);
	FGameItemTableRow* GetItemTableRow(int32 InItemIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemTable = nullptr;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//TSubclassOf<AActor> DropItemClass = nullptr;
};
