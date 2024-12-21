#pragma once

#include "TestGame/TestGame.h"
#include "MEffect.generated.h"

class UGameplayAbility;
class AWeapon;
class AMCharacter;
struct FGameplayTag;

static FGameplayTag EffectParamTag = FGameplayTag::RequestGameplayTag("Effect.Value");

UCLASS()
class TESTGAME_API UGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Damage();
};

/* 사격 후 탄약 감소 */
UCLASS()
class TESTGAME_API UGameplayEffect_ConsumeAmmo : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_ConsumeAmmo();
};

/* 리로드 */
UCLASS()
class TESTGAME_API UGameplayEffect_Reload : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Reload();
};

UCLASS()
class TESTGAME_API UGameplayEffect_AddAttribute : public UGameplayEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttribute Attribute;
};

UCLASS()
class TESTGAME_API UGameplayEffect_AddMoveSpeed : public UGameplayEffect_AddAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddMoveSpeed();
};

UCLASS()
class TESTGAME_API UGameplayEffect_AddAttackSpeed : public UGameplayEffect_AddAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddAttackSpeed();
};

UCLASS()
class TESTGAME_API UGameplayEffect_AddWeaponScale : public UGameplayEffect_AddAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddWeaponScale();
};

UCLASS()
class TESTGAME_API UGameplayEffect_AddHealth : public UGameplayEffect_AddAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddHealth();
};