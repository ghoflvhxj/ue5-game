#pragma once

#include "CoreMinimal.h"
#include "MCharacter.h"
#include "MPlayer.generated.h"

class ULevelComponent;

// 차징 공격 등의 경우 MoveBlock 태그가 부여되면서 이동 어빌리티가 막히기 때문에 Input이 캐싱이 안되는 경우가 있는데, 이 문제를 해결할 컴포넌트
UCLASS()
class TESTGAME_API UInputCacheComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInputCacheComponent();

public:
	UFUNCTION(BlueprintCallable)
	void SetInput(const FVector& InVector) { LastInputVector = InVector; }
	UFUNCTION(BlueprintPure)
	FVector GetInput() { return LastInputVector; }
protected:
	FVector LastInputVector;
};

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ULevelComponent* LevelComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputCacheComponent* InputCacheComponent = nullptr;
};

