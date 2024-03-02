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
	UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArmComponent;

	// 카메라
public:
	UFUNCTION(BlueprintCallable)
	void Test(float MeshDeltaYaw);
};