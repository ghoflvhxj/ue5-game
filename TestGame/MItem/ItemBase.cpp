#include "ItemBase.h"
#include "MGameInstance.h"

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

FItemBaseInfo* AItemBase::GetItemBaseInfo()
{
#if WITH_EDITOR
	if (IsValid(ItemDataTable))
	{
		FName ItemRowName = NAME_None;
		ItemDataTable->ForeachRow<FItemBaseInfo>(TEXT("ItemTable"), [this, &ItemRowName](const FName & Key, const FItemBaseInfo & Value) {
			if (Value.Index == ItemIndex)
			{
				ItemRowName = Key;
			}
		});

		return ItemDataTable->FindRow<FItemBaseInfo>(ItemRowName, TEXT("ItemTable"));
	}
#endif

	if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GameInstance->GetItemBaseInfo(ItemIndex);
	}

	return nullptr;
}
