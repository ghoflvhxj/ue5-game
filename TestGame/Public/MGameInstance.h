// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MGameInstance.generated.h"

UENUM()
enum class EItemType : uint8
{
	None,
	Common,
	Weapon,
	Money,
};

USTRUCT(BlueprintType)
struct FItemBaseInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UStreamableRenderAsset* GetMesh()
	{
		if (IsValid(DropMesh))
		{
			return DropMesh;
		}

		return StaticDropMesh;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UPaperSprite* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* DropMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticDropMesh = nullptr;
};

UCLASS(Blueprintable)
class TESTGAME_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

public:
	FItemBaseInfo* GetItemBaseInfo(FName InRowName);
	FItemBaseInfo* GetItemBaseInfo(int32 InItemIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable = nullptr;
	TMap<int32, FName> ItemIndexToNameMap;
};
