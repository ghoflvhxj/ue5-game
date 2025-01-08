#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "MCharacter.h"
#include "MMonster.generated.h"

class UPaperSprite;

USTRUCT(BlueprintType)
struct FMonsterInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath IconSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath IconTexture = nullptr;
};

USTRUCT(BlueprintType)
struct FMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRange = 0.f;
};

USTRUCT(BlueprintType)
struct FMonsterTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMonsterInfo MonsterInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMonsterData MonsterData;
};

UCLASS()
class TESTGAME_API AMMonster : public AMCharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* MonsterTable = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MonsterIndex = INDEX_NONE;

public:
	UFUNCTION()
	void OnStartMontageFinished(UAnimMontage* Montage, bool bInterrupted);
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bLevelStartFinished = false;

public:
	UFUNCTION(BlueprintPure)
	TSubclassOf<UGameplayAbility> GetSkill();
};

