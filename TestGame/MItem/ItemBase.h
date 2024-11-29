// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"

#include "ItemBase.generated.h"

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
protected:
	FItemBaseInfo* GetItemBaseInfo();

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable = nullptr;
#endif
};

