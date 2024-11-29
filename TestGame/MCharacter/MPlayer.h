#pragma once

#include "CoreMinimal.h"
#include "MCharacter.h"
#include "MPlayer.generated.h"

UCLASS()
class TESTGAME_API AMPlayer : public AMCharacter
{
	GENERATED_BODY()

public:
	AMPlayer();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCameraComponent* CameraComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArmComponent = nullptr;
};