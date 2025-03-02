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

UCLASS(Blueprintable)
class TESTGAME_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

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

	// 스킬 테이블
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

	// 액션 테이블
public:
	UFUNCTION(BlueprintPure)
	const FActionTableRow& GetActionTableRow(int32 InIndex);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* ActionTable = nullptr;

	// 아이템 테이블
public:
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
};
