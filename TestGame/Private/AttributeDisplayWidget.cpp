#include "AttributeDisplayWidget.h"
#include "AbilitySystemComponent.h"

bool UActorBindWidget::BindActor(AActor* InActor)
{
	if (InActor != BoundActor)
	{
		AActor* Old = BoundActor;
		BoundActor = InActor;
		OnBoundActorChanged(Old, BoundActor);

		return true;
	}

	return false;
}

void UActorBindWidget::OnBoundActorChanged_Implementation(AActor* Old, AActor* New)
{
	if (OnActorChangedEvent.IsBound())
	{
		OnActorChangedEvent.Broadcast(Old, New);
	}
}

void UAttributeDisplayWidget::OnCurrentAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	UpdateCurrentAttributeValue(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
}

void UAttributeDisplayWidget::OnMaxAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	UpdateMaxAttributeValue(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
}


void UAttributeDisplayWidget::OnBoundActorChanged_Implementation(AActor* Old, AActor* New)
{
	Super::OnBoundActorChanged_Implementation(Old, New);

	if (IsValid(Old))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Old->GetComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).Remove(CurrentDelegate);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).Remove(MaxDelegate);
			CurrentDelegate.Reset();
			MaxDelegate.Reset();
		}
	}

	if (IsValid(New))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = New->GetComponentByClass<UAbilitySystemComponent>())
		{
			CurrentDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).AddUObject(this, &UAttributeDisplayWidget::OnCurrentAttributeValueChanged);
			MaxDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UAttributeDisplayWidget::OnMaxAttributeValueChanged);

			if (CurrentAttribute.IsValid())
			{
				FOnAttributeChangeData AttributeChangeData;
				AttributeChangeData.Attribute = CurrentAttribute;
				AttributeChangeData.OldValue = 0.f;
				AttributeChangeData.NewValue = AbilitySystemComponent->GetNumericAttribute(CurrentAttribute);
				OnCurrentAttributeValueChanged(AttributeChangeData);
			}

			if (MaxAttribute.IsValid())
			{
				FOnAttributeChangeData AttributeChangeData;
				AttributeChangeData.Attribute = MaxAttribute;
				AttributeChangeData.OldValue = 0.f;
				AttributeChangeData.NewValue = AbilitySystemComponent->GetNumericAttribute(MaxAttribute);
				OnCurrentAttributeValueChanged(AttributeChangeData);
			}
		}
	}
}