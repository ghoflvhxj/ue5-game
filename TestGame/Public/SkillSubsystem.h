#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SkillSubsystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UPaperSprite;

UENUM(BlueprintType)
enum class EIGameplayEffectTarget : uint8
{
	None,
	Player,		// 아이템을 얻은 플레이어
	AllPlayer,	// 모든 플레이어
	Monster,	// 몬스터. 일반적으로 스킬 대상
	AllMonster,	// 모든 몬스터
};

USTRUCT(BlueprintType)
struct FGameplayEffectParam
{
	// 이름을 Param에서 Data로 바꾸기
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EIGameplayEffectTarget Target;

	// GameplayEffect 적용 시 사용될 파라미터. ex) Effect.Value, 50
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> MapTagToValue;
};

USTRUCT(BlueprintType)
struct FBuffInfo // 이름이 흠
{
	GENERATED_BODY()

	// DEPRECATED. 이펙트 테이블의 인덱스를 사용하도록 변경됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	// 이펙트 테이블의 인덱스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EffectIndex = INDEX_NONE;

	// DEPRECATED. 이펙트 파람을 사용하도록 변경됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EIGameplayEffectTarget Target;

	// DEPRECATED. 이펙트 파람을 사용하도록 변경됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> TagToValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectParam EffectParam;
};

USTRUCT(BlueprintType)
struct FSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSoftObjectPath Icon;

	// 에디터 전용 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	// 스킬 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;

	// 스킬 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Detail;

	// 스킬 툴팁
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Tooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FName, UAnimMontage*> NameToMontage;

	// 추가적인 GE. ex) 슬로우
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FBuffInfo> BuffInfos;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bRemoveBuffWhenSkillFinished = false;

	// 이거는 사용안함
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FGameplayTag, float> Params;				

	// 초기 파라미터. InitialParams으로 이름이 변경되야 함?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> InitialAttributes;	

	// 스킬 범위나 거리를 시각적으로 표현하는 오브젝트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftClassPath Indicator;

	// 대미지 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftObjectPath DamageSound;

	// 대미지 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftObjectPath DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftObjectPath LoadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftObjectPath ExecuteSound;

public:
	const static FSkillTableRow Empty;
};

USTRUCT(BlueprintType)
struct FSkillEnhanceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSoftObjectPath Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 CharacterIndex = INDEX_NONE;

	// 스킬 어빌리티의 태그. DEPRECATED
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag::EmptyTag;	

	// 스킬 어빌리티의 태그들.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer AbilityTags;

	// 스킬 어빌리티의 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> Data;

	// 스킬 이펙트의 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FBuffInfo> EffectParams;

public:
	const static FSkillEnhanceTableRow Empty;
};

USTRUCT(BlueprintType)
struct FSkillEnhanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> EnhanceIndices;
};


UCLASS()
class TESTGAME_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

};
