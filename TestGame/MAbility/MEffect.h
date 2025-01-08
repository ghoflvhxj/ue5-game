#pragma once

#include "TestGame/TestGame.h"
#include "MEffect.generated.h"

class UGameplayAbility;
class AWeapon;
class AMCharacter;
struct FGameplayTag;

UCLASS()
class TESTGAME_API UGameplayEffect_Base : public UGameplayEffect
{
	GENERATED_BODY()

public:
	FGameplayTag GetEffectValueTag()
	{
		return FGameplayTag::RequestGameplayTag("Effect.Value");
	}
};


UCLASS()
class TESTGAME_API UGameplayEffect_Damage : public UGameplayEffect_Base
{
	GENERATED_BODY()

public:
	UGameplayEffect_Damage();
};

/* 사격 후 탄약 감소 */
UCLASS()
class TESTGAME_API UGameplayEffect_ConsumeAmmo : public UGameplayEffect_Base
{
	GENERATED_BODY()

public:
	UGameplayEffect_ConsumeAmmo();
};

/* 리로드 */
UCLASS()
class TESTGAME_API UGameplayEffect_Reload : public UGameplayEffect_Base
{
	GENERATED_BODY()

public:
	UGameplayEffect_Reload();
};

UCLASS()
class TESTGAME_API UGameplayEffect_Attribute : public UGameplayEffect_Base
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttribute Attribute;
};

UCLASS(Abstract)
class TESTGAME_API UGameplayEffect_AddAttribute : public UGameplayEffect_Attribute
{
	GENERATED_BODY()
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

UCLASS(Abstract)
class TESTGAME_API UGameplayEffect_SetAttribute : public UGameplayEffect_Attribute
{
	GENERATED_BODY()
};

UCLASS()
class TESTGAME_API UGameplayEffect_SetHealth : public UGameplayEffect_SetAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_SetHealth();
};

UCLASS()
class TESTGAME_API UGameplayEffect_SetMaxHealth : public UGameplayEffect_SetAttribute
{
	GENERATED_BODY()

public:
	UGameplayEffect_SetMaxHealth();
};
