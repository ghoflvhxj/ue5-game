#include "ItemBase.h"
#include "MGameInstance.h"
#include "Net/UnrealNetwork.h"

DECLARE_LOG_CATEGORY_CLASS(LogItem, Log, Log);

// Sets default values
AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AItemBase, ItemIndex, COND_InitialOnly);
}

void AItemBase::SetItemIndex(int32 InItemIndex)
{
	if (HasAuthority())
	{
		ItemIndex = InItemIndex;
		OnRep_ItemIndex();
	}
}

void AItemBase::OnRep_ItemIndex()
{
	UE_LOG(LogItem, Log, TEXT("%s OnRep_ItemIndex: %d"), HasAuthority() && IsNetMode(NM_Client) == false ? TEXT("Server") : TEXT("Client"), ItemIndex);
}

const FItemBaseInfo& AItemBase::GetItemTableRowImplement()
{
	return *GetItemTableRow();
}

FItemBaseInfo* AItemBase::GetItemTableRow()
{
#if WITH_EDITOR
	if (IsValid(ItemTable))
	{
		FName ItemRowName = NAME_None;
		ItemTable->ForeachRow<FItemBaseInfo>(TEXT("ItemTable"), [this, &ItemRowName](const FName & Key, const FItemBaseInfo & Value) {
			if (Value.Index == ItemIndex)
			{
				ItemRowName = Key;
			}
		});

		return ItemTable->FindRow<FItemBaseInfo>(ItemRowName, TEXT("ItemTable"));
	}
#endif

	if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GameInstance->GetItemBaseInfo(ItemIndex);
	}

	return nullptr;
}

FGameItemData* AItemBase::GetItemData()
{
	return nullptr;
}
