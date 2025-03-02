#include "AttributeDisplayWidget.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"

bool UActorBindWidget::BindActor(AActor* InActor)
{
	if (InActor != BoundActor)
	{
		AActor* Old = BoundActor;
		BoundActor = InActor;
		
		// 액터 자동 업데이트가 꺼져있다면 자식 ActorBindWidget은 수동으로 해줘야 함
		if (IsValid(WidgetTree))
		{
			WidgetTree->ForWidgetAndChildren(GetRootWidget(), [this, InActor](UWidget* Widget) {
				if (UActorBindWidget* ActorBindWidget = Cast<UActorBindWidget>(Widget))
				{
					if (ActorBindWidget->IsAutoUpdate())
					{
						ActorBindWidget->BindActor(InActor);
					}
				}
			});
		}

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


float UAttributeDisplayWidget::GetMax()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = BoundActor->GetComponentByClass<UAbilitySystemComponent>())
	{
		if (MaxAttribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(MaxAttribute);
		}
	}

	return 0.f;
}


float UAttributeDisplayWidget::GetCurrent()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = BoundActor->GetComponentByClass<UAbilitySystemComponent>())
	{
		if (CurrentAttribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(CurrentAttribute);
		}
	}

	return 0.f;
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