#pragma once

#include "TestGame/TestGame.h"
#include "MEffect.generated.h"

class UGameplayAbility;
class AWeapon;
class AMCharacter;
struct FGameplayTag;

static FGameplayTag EffectParamTag = FGameplayTag::RequestGameplayTag("Effect.Value");

UCLASS()
class UGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Damage();
};

/* 사격 후 탄약 감소 */
UCLASS()
class UGameplayEffect_ConsumeAmmo : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_ConsumeAmmo();
};

/* 리로드 */
UCLASS()
class UGameplayEffect_Reload : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Reload();
};

UCLASS()
class UGameplayEffect_AttributeAdd : public UGameplayEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttribute Attribute;
};

UCLASS()
class UGameplayEffect_AddMoveSpeed : public UGameplayEffect_AttributeAdd
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddMoveSpeed();
};

UCLASS()
class UGameplayEffect_AddAttackSpeed : public UGameplayEffect_AttributeAdd
{
	GENERATED_BODY()

public:
	UGameplayEffect_AddAttackSpeed();
};