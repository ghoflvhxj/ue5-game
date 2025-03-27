// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "MGameInstance.generated.h"

struct FGameItemTableRow;
struct FMonsterTableRow;
struct FSkillTableRow;
struct FSkillEnhanceTableRow;
struct FActionTableRow;
struct FPlayerCharacterTableRow;
struct FDropTableRow;
struct FEffectTableRow;
struct FWeaponTableRow;

UCLASS(Blueprintable)
class TESTGAME_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void OnStart() override;
	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

protected:
	template <class T>
	T* GetTableRow(UDataTable* InTable, int32 InIndex)
	{
		if (IsValid(InTable) && InIndex != INDEX_NONE)
		{
			if (T* FoundRow = InTable->FindRow<T>(*FString::FromInt(InIndex), TEXT("MonsterTable")))
			{
				return FoundRow;
			}
		}

		return nullptr;
	}

	// 몬스터 테이블
public:
	UFUNCTION(BlueprintPure)
	const FMonsterTableRow& GetMonsterTableRow(int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* MonsterTable = nullptr;

	// 무기 테이블
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "Context"))
	static const FWeaponData& GetWeaponTableRow(UObject* Context, int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* WeaponTable = nullptr;

	// 스킬 테이블
public:
	static void LoadSkillAsset(UObject* WorldContext, int32 InSkillIndex, bool bIncludeChildren = false);
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "Context"))
	static const FSkillTableRow& GetSkillTableRow(UObject* Context, int32 InIndex);
	TArray<FSkillTableRow> GetSkillTableRowsByPredicate(TFunction<bool(const FSkillTableRow&)> Pred);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* SkillTable = nullptr;

	// 스킬 강화 테이블
public:
	UFUNCTION(BlueprintPure)
	const FSkillEnhanceTableRow& GetSkillEnhanceTableRow(int32 InIndex);
	TArray<int32> GetSkillEnhanceTableRowsByPredicate(TFunction<bool(const FSkillEnhanceTableRow&)> Pred);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* SkillEnhanceTable = nullptr;

	// 이펙트 테이블
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static const FEffectTableRow& GetEffectTableRow(const UObject* WorldContextObject, int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* EffectTable = nullptr;

	// 액션 테이블
public:
	UFUNCTION(BlueprintPure)
	const FActionTableRow& GetActionTableRow(int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* ActionTable = nullptr;

	// 아이템 테이블
public:
	TArray<FGameItemTableRow*> FilterItemByPredicate(TFunction<bool(const FGameItemTableRow* const InGameItemTableRow)> Func);
	void IterateItemTable(TFunction<void(const FGameItemTableRow& GameItemTableRow)> Function);
	FGameItemTableRow* GetItemTableRow(FName InRowName);
	FGameItemTableRow* GetItemTableRow(int32 InItemIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemTable = nullptr;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//TSubclassOf<AActor> DropItemClass = nullptr;

	// 드랍 테이블
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "Context"))
	static const FDropTableRow& GetDropTableRow(UObject* Context, int32 InIndex);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* DropTable = nullptr;

	// 플레이어 캐릭터 테이블
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "Context"))
	static const FPlayerCharacterTableRow& GetPlayerCharacterTableRow(UObject* Context, int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* PlayerCharacterTable = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void SetLoadWidget(UUserWidget* InWidget) { LoadWidget = InWidget; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> LoadWidgetClass = nullptr;
	UUserWidget* LoadWidget = nullptr;
	UFUNCTION(BlueprintCallable)
	void OpenLevel(FName InLevelName);

};
