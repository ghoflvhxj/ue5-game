// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"

#include "ItemBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;

UENUM()
enum class EItemType : uint8
{
	None,
	Common,
	Weapon,
	Money,
};

UENUM(BlueprintType)
enum class EItemGrade : uint8
{
	None,
	Normal,
	Rare,
	Unique,
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
	TSet<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMAbilityDataAsset* AbilitySet;

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
	EItemGrade Grade = EItemGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UPaperSprite* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath DropMesh = nullptr;

public:
	const FSoftObjectPath& GetWorldAssetPath() const
	{
		return DropMesh; // DropMesh이름을 변경하면 영향이 많으니 일단 이렇게
	}
};


USTRUCT(BlueprintType)
struct FGameItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	static const FGameItemTableRow Empty;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameItemInfo GameItemInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameItemData GameItemData;

public:
#if WITH_EDITOR
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override
	{
		Super::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
		if (InRowName.ToString().IsNumeric())
		{
			Index = FCString::Atoi(*InRowName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DataTable(%s) rowname(%s) is not numeric. Failed to initialize index property!!!"), *InDataTable->GetName(), *InRowName.ToString());
		}
	}
#endif

	template<class T>
	T* GetWorldAsset() const
	{
		if (GameItemInfo.GetWorldAssetPath().IsValid())
		{
			return Cast<T>(GameItemInfo.GetWorldAssetPath().TryLoad());
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
	const FGameItemTableRow& GetItemTableRowImplement();
	FGameItemTableRow* GetItemTableRow();
	// 아이템 데이터
public:
	FGameItemData* GetItemData();

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemTable = nullptr;
#endif
};

