#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MEffectTypes.generated.h"

USTRUCT()
struct TESTGAME_API FMGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	virtual FGameplayEffectContext* Duplicate() const override;

public:
	UPROPERTY()
	int32 EffectIndex = INDEX_NONE;
};

template<>
struct TStructOpsTypeTraits<FMGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

USTRUCT(BlueprintType)
struct TESTGAME_API FMGameplayEffectContextHandle : public FGameplayEffectContextHandle
{
	GENERATED_BODY()

public:
	FMGameplayEffectContextHandle();
	FMGameplayEffectContextHandle(FGameplayEffectContext* DataPtr);
	virtual ~FMGameplayEffectContextHandle() {}

public:
	void SetEffectIndex(int32 InEffectindex);
	static FMGameplayEffectContext* GetMEffectContext(FGameplayEffectContextHandle& InContextHandle);
};

UCLASS()
class TESTGAME_API UMGameplayEffectContextHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static int32 GetEffectIndex(FGameplayEffectContextHandle InContextHandle);

	static void SetEffectIndex(FGameplayEffectContextHandle& InContextHandle, int32 InEffectIndex);
};