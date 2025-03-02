#include "MGameInstance.h"

#include "TestGame/MItem/Drop.h"
#include "TestGame/MItem/ItemBase.h"
#include "TestGame/MCharacter/MMonster.h"
#include "TestGame/MAbility/MActionStruct.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "SkillSubsystem.h"

void UMGameInstance::Init()
{
	Super::Init();

	if (IsValid(ItemTable))
	{
		//TArray<FItemBaseInfo*> ItemBaseInfos;
		//ItemDataTable->GetAllRows(TEXT("ItemTable"), ItemBaseInfos);

		//for (const FItemBaseInfo* const ItemBaseInfo : ItemBaseInfos)
		//{
		//	ItemIndexToNameMap.Emplace(ItemBaseInfo->Index, ItemBaseInfo->);
		//}
	}
}

const FMonsterTableRow& UMGameInstance::GetMonsterTableRow(int32 InIndex)
{
	if (FMonsterTableRow* TableRow = GetTableRow<FMonsterTableRow>(MonsterTable, InIndex))
	{
		return *TableRow;
	}

	return FMonsterTableRow::Empty;
}


const FSkillTableRow& UMGameInstance::GetSkillTableRow(UObject* Context, int32 InIndex)
{
	const FSkillTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FSkillTableRow>(GameInstance->SkillTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FSkillTableRow::Empty;
	}

	return *Out;
}


TArray<FSkillTableRow> UMGameInstance::GetSkillTableRowsByPredicate(TFunction<bool(const FSkillTableRow&)> Pred)
{
	TArray<FSkillTableRow*> SkillTableRows;
	if (IsValid(SkillTable))
	{
		SkillTable->GetAllRows<FSkillTableRow>(TEXT("SkillTable"), SkillTableRows);
	}

	TArray<FSkillTableRow> OutRows;
	for (FSkillTableRow* SkillTableRow : SkillTableRows)
	{
		if (Pred(*SkillTableRow))
		{
			OutRows.Add(*SkillTableRow);
		}
	}

	return OutRows;
}


TArray<int32> UMGameInstance::GetSkillEnhanceTableRowsByPredicate(TFunction<bool(const FSkillEnhanceTableRow&)> Pred)
{
	TArray<FSkillEnhanceTableRow*> SkillTableRows;
	if (IsValid(SkillEnhanceTable))
	{
		SkillEnhanceTable->GetAllRows<FSkillEnhanceTableRow>(TEXT("SkillEnhanceTable"), SkillTableRows);
	}

	TArray<int32> OutRows;
	for (FSkillEnhanceTableRow* SkillTableRow : SkillTableRows)
	{
		if (Pred(*SkillTableRow))
		{
			OutRows.Add(SkillTableRow->Index);
		}
	}

	return OutRows;
}

const FActionTableRow& UMGameInstance::GetActionTableRow(int32 InIndex)
{
	if (FActionTableRow* TableRow = GetTableRow<FActionTableRow>(ActionTable, InIndex))
	{
		return *TableRow;
	}

	return FActionTableRow::Empty;
}

void UMGameInstance::IterateItemTable(TFunction<void(const FGameItemTableRow& GameItemTableRow)> Function)
{
	if (IsValid(ItemTable) == false)
	{
		return;
	}

	TArray<FGameItemTableRow*> GameItemTableRows;
	ItemTable->GetAllRows<FGameItemTableRow>(TEXT("ItemTable"), GameItemTableRows);

	for (FGameItemTableRow* GameItemTableRow : GameItemTableRows)
	{
		Function(*GameItemTableRow);
	}
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(int32 InIndex)
{
	return GetItemTableRow(*FString::FromInt(InIndex));
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(FName InRowName)
{
	if (IsValid(ItemTable) && InRowName != NAME_None)
	{
		return ItemTable->FindRow<FGameItemTableRow>(InRowName, TEXT("ItemTable"));
	}

	return nullptr;
}


const FDropTableRow& UMGameInstance::GetDropTableRow(UObject* Context, int32 InIndex)
{
	const FDropTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FDropTableRow>(GameInstance->DropTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FDropTableRow::Empty;
	}

	return *Out;
}

const FSkillEnhanceTableRow& UMGameInstance::GetSkillEnhanceTableRow(int32 InIndex)
{
	if (FSkillEnhanceTableRow* TableRow = GetTableRow<FSkillEnhanceTableRow>(SkillEnhanceTable, InIndex))
	{
		return *TableRow;
	}

	return FSkillEnhanceTableRow::Empty;
}

const FPlayerCharacterTableRow& UMGameInstance::GetPlayerCharacterTableRow(UObject* Context, int32 InIndex)
{
	const FPlayerCharacterTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FPlayerCharacterTableRow>(GameInstance->PlayerCharacterTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FPlayerCharacterTableRow::Empty;
	}
	
	return *Out;
}
