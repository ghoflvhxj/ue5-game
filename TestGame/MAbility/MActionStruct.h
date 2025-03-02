#pragma once

#include "MActionStruct.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class EActionConditionOp : uint8
{
	None,
	Equal,
	NotEqual,
	Lower,
	EqualLower,
	Greater,
	EqualGreater,
	Max
};

USTRUCT(BlueprintType)
struct FActionCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Condition = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EActionConditionOp Operator = EActionConditionOp::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//FName Target;
};

USTRUCT(BlueprintType)
struct FActionTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FActionCondition ActionCondition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillIndex = INDEX_NONE;

public:
	const static FActionTableRow Empty;
};