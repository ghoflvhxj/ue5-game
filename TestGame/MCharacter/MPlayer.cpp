#include "MPlayer.h"
#include "CharacterLevelSubSystem.h"

AMPlayer::AMPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	LevelComponent = CreateDefaultSubobject<ULevelComponent>(TEXT("LevelComponent"));

	InputCacheComponent = CreateDefaultSubobject<UInputCacheComponent>(TEXT("InputCacheComponent"));
}

UInputCacheComponent::UInputCacheComponent()
{
	SetIsReplicatedByDefault(false);
	PrimaryComponentTick.bCanEverTick = false;
}
