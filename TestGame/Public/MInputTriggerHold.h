#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "MInputTriggerHold.generated.h"

UCLASS()
class TESTGAME_API UMInputTriggerHold : public UInputTriggerTimedBase
{
	GENERATED_BODY()

	bool bTriggered = false;

protected:
	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;

public:
	virtual ETriggerEventsSupported GetSupportedTriggerEvents() const override { return ETriggerEventsSupported::Ongoing; }

	// How long does the input have to be held to cause trigger?
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings", meta = (ClampMin = "0"))
	float HoldTimeThreshold = 1.0f;

	// Should this trigger fire only once, or fire every frame once the hold time threshold is met?
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings")
	bool bIsOneShot = false;

	virtual FString GetDebugState() const override { return HeldDuration ? FString::Printf(TEXT("Hold:%.2f/%.2f"), HeldDuration, HoldTimeThreshold) : FString(); }

public:
	DECLARE_DELEGATE_RetVal(bool, FOnTestDelegate);
	FOnTestDelegate OnTestDelegate;
};
