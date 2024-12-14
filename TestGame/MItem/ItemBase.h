// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"

#include "ItemBase.generated.h"

class UGameplayEffect;

UENUM()
enum class EItemType : uint8
{
	None,
	Common,
	Weapon,
	Money,
};

USTRUCT(BlueprintType)
struct FGameplayEffectParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> ParamTagToValueMap;
};

USTRUCT(BlueprintType)
struct FGameItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TSubclassOf<UGameplayEffect>, FGameplayEffectParam> GameplayEffects;
};

USTRUCT(BlueprintType)
struct FGameItemInfo
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UPaperSprite* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath DropMesh = nullptr;
};


USTRUCT(BlueprintType)
struct FItemBaseInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameItemInfo GameItemInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameItemData GameItemData;

public:
	UStreamableRenderAsset* GetMesh() const
	{
		if (GameItemInfo.DropMesh.IsValid())
		{
			return Cast<UStreamableRenderAsset>(GameItemInfo.DropMesh.TryLoad());
		}
		return nullptr;
	}
};

UCLASS()
class TESTGAME_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable)
	void SetItemIndex(int32 InItemIndex);
	UFUNCTION()
	virtual void OnRep_ItemIndex();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ItemIndex)
	int32 ItemIndex = INDEX_NONE;

	// 아이템 정보
public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetItemTableRow"))
	const FItemBaseInfo& GetItemTableRowImplement();
	FItemBaseInfo* GetItemTableRow();
	// 아이템 데이터
public:
	FGameItemData* GetItemData();

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemTable = nullptr;
#endif
};

