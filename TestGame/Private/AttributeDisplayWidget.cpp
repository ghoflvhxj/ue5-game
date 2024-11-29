#include "AttributeDisplayWidget.h"
#include "AbilitySystemComponent.h"

void UAttributeDisplayWidget::OnCurrentAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	UpdateCurrentAttributeValue(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
}


void UAttributeDisplayWidget::OnMaxAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	UpdateMaxAttributeValue(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
}

void UAttributeDisplayWidget::BindAttributeOwner(AActor* InAttributeOwner)
{
	if (IsValid(InAttributeOwner) == false || AttributeOwner == InAttributeOwner)
	{
		return;
	}

	if (IsValid(AttributeOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AttributeOwner->GetComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).Remove(CurrentDelegate);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).Remove(MaxDelegate);
		}
	}

	AttributeOwner = InAttributeOwner;
	if (UAbilitySystemComponent* AbilitySystemComponent = AttributeOwner->GetComponentByClass<UAbilitySystemComponent>())
	{
		CurrentDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).AddUObject(this, &UAttributeDisplayWidget::OnCurrentAttributeValueChanged);
		MaxDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UAttributeDisplayWidget::OnMaxAttributeValueChanged);
	}

	OnAttributeOwnerChanged(AttributeOwner);
}
