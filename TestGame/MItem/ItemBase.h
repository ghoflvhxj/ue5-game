// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"

#include "ItemBase.generated.h"

class UPaperSprite;

UENUM()
enum class EItemType : uint8
{
	None,
	Common,
	Weapon,
};

USTRUCT(BlueprintType)
struct FItemBaseInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPaperSprite* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* DropMesh = nullptr;
};

UCLASS()
class TESTGAME_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 아이템 정보
public:
	UFUNCTION(BlueprintCallable)
	void SetItemIndex(int32 InItemIndex);
	UFUNCTION()
	virtual void OnRep_ItemIndex();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ItemIndex)
	int32 ItemIndex = INDEX_NONE;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable = nullptr;
protected:
	FItemBaseInfo* GetItemBaseInfo();
};

