#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "AttributeDisplayWidget.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class TESTGAME_API UAttributeDisplayWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentAttributeValue(float InOldValue, float InNewvalue);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMaxAttributeValue(float InOldValue, float InNewvalue);
	void OnCurrentAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData);
	void OnMaxAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData);

public:
	UFUNCTION(BlueprintCallable)
	void BindAttributeOwner(AActor* InAttributeOwner);
	UFUNCTION(BlueprintImplementableEvent)
	void OnAttributeOwnerChanged(AActor* InAttributeOwner);
protected:
	UPROPERTY(BlueprintReadOnly)
	AActor* AttributeOwner = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute CurrentAttribute;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute MaxAttribute;
	FDelegateHandle CurrentDelegate;
	FDelegateHandle MaxDelegate;
};
