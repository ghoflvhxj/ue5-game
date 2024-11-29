#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OffScreenIndicator.generated.h"

USTRUCT(BlueprintType)
struct FIndicatorData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Target = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

public:
	bool operator==(const FIndicatorData& Rhs)
	{
		return Target != nullptr && Target == Rhs.Target;
	}
};

UCLASS()
class TESTGAME_API UOffScreenIndicateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowIndicateWidget();
	UFUNCTION(BlueprintNativeEvent)
	void HideIndicateWidget(bool bRemove = false);
protected:
	UPROPERTY(BlueprintReadOnly)
	float Angle = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

public:
	UFUNCTION(BlueprintCallable)
	void SetIndicateTarget(const FIndicatorData& InIndicateData);
protected:
	UPROPERTY(BlueprintReadOnly)
	FIndicatorData IndicateData;
	FDelegateHandle Handle;
};
