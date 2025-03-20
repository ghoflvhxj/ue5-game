#pragma once

#include "MActionStruct.h"
#include "MAbility.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MItem/ItemBase.h" // 임시 FGamepalyEffectParam용
#include "MActionAbility.generated.h"

struct FSkillTableRow;
class AMCharacter;
class ABullet;
class UGameplayEffect_Damage;

// Ability_Action으로 바꾸기?
UCLASS()
class TESTGAME_API UGameplayAbility_Skill : public UGameplayAbility_CharacterBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Skill();

public:
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	// 스킬이 로드 시 수행할 내용을 여기에 추가함.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnLoad();
	// 스킬이 무엇을 할지 여기서 작성함.
	UFUNCTION(BlueprintNativeEvent)
	void OnActive();

protected:
	void ExecuteSoundCue(FGameplayTag InTag);

/* 스킬 인덱스와 테이블 관련 */
public:
	void SetSkillIndex(int32 InIndex, FGameplayTag InSkillTag);
	UFUNCTION(BlueprintPure)
	const FSkillTableRow& GetSkillTableRow();
	void SkillEnhance(int32 SkillEnhanceIndex);
	UFUNCTION()
	void OnRep_SkillIndex();
protected:
	//UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	UPROPERTY(ReplicatedUsing = OnRep_SkillIndex, VisibleAnywhere, BlueprintReadOnly)
	int32 SkillIndex = INDEX_NONE;

public:
	UFUNCTION(BlueprintPure)
	FGameplayTag GetSkillTag() const { return SkillTag; }
protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	FGameplayTag SkillTag = FGameplayTag::EmptyTag;

protected:
	// 어빌리티를 즉시 끝냄. 해제 시 수동으로 EndAbility를 반드시 해줘야 함
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEndInstantly = true;

/* GE 관련 */
public:
	virtual void ApplyBuffByNoneEvent() override;
	virtual void ApplyBuffByDamageEvent(AActor* InEffectCauser, AActor* InTarget) override;
protected:
	// GE에 대한 파라미터 ex) 슬로우 수치, 시간 등
	TMap<int32, FGameplayEffectParam> MapEffectToParams;
	// 테이블에 설정된 버프 이펙트 핸들
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
	// 어빌리티가 끝날 때 같이 종료될 버프 이펙트 핸들
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> PendingRemoveEffectHandles;

/* Duration 관련 */
public:
	UFUNCTION(BlueprintPure)
	FActiveGameplayEffectHandle GetDurationEffectHandle() { return DurationEffectHandle; }
protected:
	UPROPERTY(EditDefaultsOnly)
	int32 DurationEffectIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle DurationEffectHandle;

	// Duration 후 적용할 쿨타임. 스킬 완료 후 적용되는 쿨타임
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> SubCoolDownEffectClass = nullptr;

	FTimerHandle DurationTimerHandle;

/* 스킬 인디케이터 */
public:
	// 테이블에서 인디케이터가 있다면 생성함
	UFUNCTION(BlueprintCallable)
	void SpawnIndicator();
	UFUNCTION(BlueprintCallable)
	void RemoveIndicator();
protected:
	TWeakObjectPtr<AActor> Indicator = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Dash : public UGameplayAbility_Skill
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();

public:
	virtual void OnActive_Implementation() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UFUNCTION()
	void AddMovementInput(int32 ActionIndex);
	UFUNCTION()
	void ClearDash(int32 ActionIndex);
	UFUNCTION()
	void StopDash(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void OnAnimFinished();

protected:
	FVector StartLocation = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DashLength = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequenceBase> Anim;

	class UAbilityTask_Repeat* RepeatTask = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_BattoSkill : public UGameplayAbility_Skill
{
	GENERATED_BODY()

public:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

public:
	virtual void OnActive_Implementation() override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_SwordWave : public UGameplayAbility_Skill
{
	GENERATED_BODY()

public:
	UGameplayAbility_SwordWave();

public:
	virtual void OnActive_Implementation() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	FDelegateHandle WeaponChangeDelegateHandle;
	FDelegateHandle WeaponComboChangeDelegateHandle;

protected:
	void SpawnSwordWave();
	void BindToWeaponCombo();
protected:
	TWeakObjectPtr<AWeapon> BoundWeapon = nullptr;
	FTimerHandle TimerHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABullet> BulletClass = nullptr;
};