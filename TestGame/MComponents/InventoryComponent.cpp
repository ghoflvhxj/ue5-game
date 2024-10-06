#include "InventoryComponent.h"

UMInventoryComponent::UMInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMInventoryComponent, Money);
}

bool UMInventoryComponent::AddMoney(int32 InAdditiveMoney)
{
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