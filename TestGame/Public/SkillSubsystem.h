#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SkillSubsystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FSkillAbilities
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	//UPROPERTY()
};

USTRUCT(BlueprintType)
struct FBuffInfo // FSkillBuffs  로 변경하기
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> TagToValue;
};

USTRUCT(BlueprintType)
struct FSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage = nullptr;	// Deprecated

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, UAnimMontage*> NameToMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBuffInfo> BuffInfos;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRemoveBuffWhenSkillFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> Test;			// Deprecated

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> Params;
};

UCLASS()
class TESTGAME_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	
	
	
};
