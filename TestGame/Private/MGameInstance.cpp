#include "MGameInstance.h"


void UMGameInstance::Init()
{
	Super::Init();

	if (IsValid(ItemDataTable))
	{
		ItemDataTable->ForeachRow<FItemBaseInfo>(TEXT("ItemTable"), [this](const FName & Key, const FItemBaseInfo & Value) {
			ItemIndexToNameMap.Emplace(Value.Index, Key);
		});

		//TArray<FItemBaseInfo*> ItemBaseInfos;
		//ItemDataTable->GetAllRows(TEXT("ItemTable"), ItemBaseInfos);

		//for (const FItemBaseInfo* const ItemBaseInfo : ItemBaseInfos)
		//{
		//	ItemIndexToNameMap.Emplace(ItemBaseInfo->Index, ItemBaseInfo->);
		//}
	}
}

FItemBaseInfo* UMGameInstance::GetItemBaseInfo(int32 InItemIndex)
{
	if (InItemIndex != INDEX_NONE && ItemIndexToNameMap.Contains(InItemIndex))
	{
		return GetItemBaseInfo(ItemIndexToNameMap[InItemIndex]);
	}

	return nullptr;
}

FItemBaseInfo* UMGameInstance::GetItemBaseInfo(FName InRowName)
{
	if (IsValid(ItemDataTable) && InRowName != NAME_None)
	{
		return ItemDataTable->FindRow<FItemBaseInfo>(InRowName, TEXT("ItemTable"));
	}

	return nullptr;
}
