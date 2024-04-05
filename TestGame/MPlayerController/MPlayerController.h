#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPlayerController.generated.h"

UCLASS()
class TESTGAME_API AMPlayerController : public APlayerController
{
	GENERATED_BODY()
};

UCLASS()
class TESTGAME_API AMPlayerControllerTitle : public AMPlayerController
{
	GENERATED_BODY()

public:
	//virtual void BeginPlay() override;
};


UCLASS()
class TESTGAME_API AMPlayerControllerInGame : public AMPlayerController
{
	GENERATED_BODY()

public:
	AMPlayerControllerInGame();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	FVector GetMouseWorldPosition();

protected:
	UFUNCTION(BlueprintNativeEvent)
	void AddInputMappingContext();
};