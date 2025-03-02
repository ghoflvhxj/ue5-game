// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "SkillSubsystem.h"

#include "ItemBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UMAbilityDataAsset;

UENUM()
enum class EItemEvent : uint8
{
	None,
	Pause,
};

UENUM()
enum class EItemType : uint8
{
	None,
	Common,
	Item,
	Weapon,
	Money,
	Exp,
};

UENUM(BlueprintType)
enum class EItemGrade : uint8
{
	None,
	Normal,
	Rare,
	Unique,
};

// 아이템 레벨에 따른 파라미터 설정 테이블을 위해 만들었음, 범용적으로 사용하기 위해 이름을 추상적으로 만듬. 
USTRUCT(BlueprintType)
struct FGameplayParamsTableRow : public FTableRowBase							
{
	GENERATED_BODY();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> ParameterNameToValueMap;
};

USTRUCT(BlueprintType)
struct FGameItemData
{
	GENERATED_BODY()

public:
	// 아이템 획득자에 추가되는 어빌리티 세트. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMAbilityDataAsset* AbilitySet = nullptr;									

	// 아이템이 주는 Abilities의 초기 파라미터. 추후 Abiliity테이블을 만들어서 옮겨야 할 듯. DEPRECATED
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> InitialParams;									

	// 아이템 획득 시 즉시 적용되는 GameplayEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<TSubclassOf<UGameplayEffect>, FGameplayEffectParam> GameplayEffects;	

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//TMap<EItemEvent, float> Events;

	// 아이템 어빌리티의 파라미터 테이블. 행은 레벨, 로우는 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ParamTable = nullptr;											


public:
	TMap<FGameplayTag, float> GetParam(int32 InLevel = 1)
	{
		if (IsValid(ParamTable))
		{
			if (FGameplayParamsTableRow* ParamTableRow = ParamTable->FindRow<FGameplayParamsTableRow>(*FString::FromInt(InLevel), TEXT("AbilityParam")))
			{
				return ParamTableRow->ParameterNameToValueMap;
			}
		}
		else if (InLevel == 1)
		{
			return InitialParams;
		}

		return TMap<FGameplayTag, float>();
	}
	int32 GetMaxLevel() const
	{
		if (IsValid(ParamTable))
		{
			return ParamTable->GetRowMap().Num();
		}

		return 1;
	}
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
	FSoftObjectPath Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath DropMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftClassPath DropItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath AcquireSound;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;									// 개발자 전용 설명

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

