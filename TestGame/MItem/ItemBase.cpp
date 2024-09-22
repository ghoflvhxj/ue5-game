#include "ItemBase.h"

// Sets default values
AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemBase::SetItemIndex(int32 InItemIndex)
{
	ItemIndex = InItemIndex;
}

FItemBaseInfo* AItemBase::GetItemBaseInfo()
{
	if (IsValid(ItemDataTable) && ItemIndex != INDEX_NONE)
	{
		return ItemDataTable->FindRow<FItemBaseInfo>(*FString::FromInt(ItemIndex), TEXT("ItemTable"));
	}

	return nullptr;
}
