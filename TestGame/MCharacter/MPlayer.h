#pragma once

#include "EngineMinimal.h"
#include "MCharacter.h"
#include "EnhancedInputComponent.h"
#include "MPlayer.generated.h"

class ULevelComponent;
class UInputComponent;
class UInputAction;
class UGameplayAbility_Skill;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillActivatedDelegate, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillEnhancedDelegate, int32);

USTRUCT(BlueprintType)
struct FPlayerCharacterTableRow : public FTableRowBase
{
	GENERATED_BODY()

	const static FPlayerCharacterTableRow Empty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	// 에디터 전용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Detail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AMPlayer> PlayerCharacterClass = nullptr;

	// 스킬과 어빌리티 태그의 연결. 이름을 고민해봐야 함
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, FGameplayTag> Skills;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath InitialAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Icon;
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

protected:
	virtual void BeginPlay() override;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPlayerDefaults() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	virtual void BasicAttack() override;

public:
	virtual void OnStartAnimFinished_Implementation(UAnimMontage* Montage, bool bInterrupted) override;

public:
	virtual int32 GetCharacterIndex() override;

	// 시작 애님 관련
public:
	UFUNCTION()
	void StopStartAnim();
protected:
	FEnhancedInputActionEventBinding* InputHandle = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* StartInputAction = nullptr;

	// 스킬 로드&사용
public:
	UFUNCTION(BlueprintPure)
	UGameplayAbility* GetLoadedSkillAbility();
	UGameplayAbility_Skill* GetSkillAbility(int32 InSkillIndex);
	UFUNCTION(BlueprintCallable)
	void UseSkill(int32 InSkillSlot);
	UFUNCTION(BlueprintCallable, meta=(DeprecatedFunction))
	void FinishSkill();
	FOnSkillActivatedDelegate& GetSkillActivatedDelegate() { return OnSkillActivatedDelegate; }
	FOnSkillActivatedDelegate OnSkillActivatedDelegate;
	void SetLoadedSkillIndex(int32 InSkillIndex) { LoadedSkillIndex = InSkillIndex; }
protected:
	int32 LoadedSkillIndex = INDEX_NONE;

	//스킬 강화
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SkillEnhance(int32 InIndex);
	FOnSkillEnhancedDelegate& GetSkillEnhancedDelegate() { return OnSkillEnhancedDelegate; }
protected:
	FOnSkillEnhancedDelegate OnSkillEnhancedDelegate;

/* GAS Effect */
public:
	void UpdateGameplayEffect(UAbilitySystemComponent* InAbilitySystemCoponent, const FGameplayEffectSpec& InGameplayEffectSpec, FActiveGameplayEffectHandle InActiveGameplayEffectHandle);
	void RemoveGameplayEffect(const FActiveGameplayEffect& InRemovedGameplayEffect);
};

// 차징 공격 등의 경우 MoveBlock 태그가 부여되면서 이동 어빌리티가 막히기 때문에 Input이 캐싱이 안되는 경우가 있는데, 이 문제를 해결할 컴포넌트
UCLASS()
class TESTGAME_API UInputCacheComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInputCacheComponent();

public:
	UFUNCTION(BlueprintCallable)
	void SetInput(const FVector& InVector);
	UFUNCTION(BlueprintPure)
	FVector GetInput() { return LastInputVector; }
	UFUNCTION(BlueprintCallable)
	void UpdateStack(const FVector& InVector);
	UFUNCTION(BlueprintGetter)
	int GetStackCount();
protected:
	FVector LastInputVector;
	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetStackCount)
	int Stack = 0;
};