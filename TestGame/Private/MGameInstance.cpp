#include "MGameInstance.h"

#include "TestGame/MItem/ItemBase.h"

void UMGameInstance::Init()
{
	Super::Init();

	if (IsValid(ItemDataTable))
	{
		//TArray<FItemBaseInfo*> ItemBaseInfos;
		//ItemDataTable->GetAllRows(TEXT("ItemTable"), ItemBaseInfos);

		//for (const FItemBaseInfo* const ItemBaseInfo : ItemBaseInfos)
		//{
		//	ItemIndexToNameMap.Emplace(ItemBaseInfo->Index, ItemBaseInfo->);
		//}
	}
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(int32 InItemIndex)
{
	return GetItemTableRow(*FString::FromInt(InItemIndex));
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(FName InRowName)
{
	if (IsValid(ItemDataTable) && InRowName != NAME_None)
	{
		return ItemDataTable->FindRow<FGameItemTableRow>(InRowName, TEXT("ItemTable"));
	}

	return nullptr;
}
