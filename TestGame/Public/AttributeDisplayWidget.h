#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "AttributeDisplayWidget.generated.h"

struct FOnAttributeChangeData;

DECLARE_EVENT_TwoParams(UActorBindWidget, FOnActorChangedEvent, AActor*, AActor*);

UCLASS()
class TESTGAME_API UActorBindWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	bool BindActor(AActor* InActor);
	UFUNCTION(BlueprintNativeEvent)
	void OnBoundActorChanged(AActor* Old, AActor* New);
	FOnActorChangedEvent& GetActorChangedEvent() { return OnActorChangedEvent; }
protected:
	UPROPERTY(BlueprintReadOnly)
	AActor* BoundActor = nullptr;
	FOnActorChangedEvent OnActorChangedEvent;
};

UCLASS()
class TESTGAME_API UAttributeDisplayWidget : public UActorBindWidget
{
	GENERATED_BODY()

public:
	virtual void OnBoundActorChanged_Implementation(AActor* Old, AActor* New) override;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentAttributeValue(float InOldValue, float InNewvalue);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMaxAttributeValue(float InOldValue, float InNewvalue);
	
	void OnCurrentAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData);
	void OnMaxAttributeValueChanged(const FOnAttributeChangeData& AttributeChangeData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute CurrentAttribute;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute MaxAttribute;
	FDelegateHandle CurrentDelegate;
	FDelegateHandle MaxDelegate;
};
