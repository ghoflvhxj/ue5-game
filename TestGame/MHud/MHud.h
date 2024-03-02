#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Hud.h"

#include "MHud.generated.h"

UCLASS()
class TESTGAME_API AMHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowGameOver();
};