#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "TestGame/MFunctionLibrary/MContainerFunctionLibrary.h"

UMInventoryComponent::UMInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMInventoryComponent, Money);
	DOREPLIFETIME(UMInventoryComponent, SerializeItems);
}

bool UMInventoryComponent::AddMoney(int32 InAdditiveMoney)
{
	if (IsNetSimulating())
	{
		return false;
	}

	if (Money + InAdditiveMoney < 0)
	{
		return false;
	}

	Money += InAdditiveMoney;
	OnRep_Money();

	return true;
}

void UMInventoryComponent::OnRep_Money()
{
	OnMoneyChangedEvent.Broadcast(Money);
}

void UMInventoryComponent::AddItem(int32 InIndex, int32 InNum)
{
	// 데디 서버, 리슨 서버, 스탠드얼론 통과
	if (IsNetSimulating())
	{
		return;
	}

	ItemMap.FindOrAdd(InIndex) += InNum;

	// 스탠드얼론은 네트워크가 아니니 제외
	if (IsNetMode(NM_Standalone) == false)
	{
		UContainerFunctionLibrary::SerializeMap(ItemMap, SerializeItems);
	}

	OnRep_Items();
}

void UMInventoryComponent::OnRep_Items()
{
	if (IsNetMode(NM_Standalone) == false)
	{
		UContainerFunctionLibrary::DeserializeMap(ItemMap, SerializeItems);
	}

	TMap<int32, int32> ItemDiffMap;
	for (auto& Pair : ItemMap)
	{
		int32 ItemIndex = Pair.Key;
		int32 Delta = 0;
		if (CachedItemMap.Contains(ItemIndex) == false)
		{
			Delta = Pair.Value;
		}
		else
		{
			Delta = ItemMap[ItemIndex] > CachedItemMap[ItemIndex];
		}

		if (Delta > 0)
		{
			OnItemAddedEvent.Broadcast(ItemIndex, Delta);
		}
	}

	CachedItemMap = ItemMap;
}

void UMInventoryComponent::SerializeMap(FArchive& Archive)
{
	if (Archive.IsSaving())
	{
		int32 Num = ItemMap.Num();
		Archive << Num;

		for (auto& Elem : ItemMap)
		{
			Archive << Elem.Key;
			Archive << Elem.Value;
		}
	}
	else if (Archive.IsLoading())
	{
		ItemMap.Empty();
		int32 Num;
		Archive << Num;

		for (int32 i = 0; i < Num; ++i)
		{
			int32 Key;
			int32 Value;
			Archive << Key;
			Archive << Value;
			ItemMap.FindOrAdd(Key) = Value;
		}
	}
}
