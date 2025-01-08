#include "MGameInstance.h"

#include "TestGame/MItem/ItemBase.h"

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

FGameItemTableRow* UMGameInstance::GetItemTableRow(int32 InItemIndex)
{
	return GetItemTableRow(*FString::FromInt(InItemIndex));
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(FName InRowName)
{
	if (IsValid(ItemTable) && InRowName != NAME_None)
	{
		return ItemTable->FindRow<FGameItemTableRow>(InRowName, TEXT("ItemTable"));
	}

	return nullptr;
}
