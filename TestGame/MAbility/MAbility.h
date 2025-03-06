// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "TestGame/MAbility/MAbilitySystemComponent.h"
#include "MAbility.generated.h"

class UGameplayAbility;
class AWeapon;
class AMCharacter;
class UGameplayEffect_Damage;

struct FGameplayTag;

UINTERFACE(BlueprintType)
class TESTGAME_API UActorByAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IActorByAbilityInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitUsingAbility(UGameplayAbility* InAbility);
};


USTRUCT(BlueprintType)
struct TESTGAME_API FMAbilityBindInfo
{
	GENERATED_BODY();

public:
	// 어빌리티의 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GameplayTag = FGameplayTag::EmptyTag;		

	// 어빌리티 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> GameplayAbilityClass = nullptr;	

	// InputID 만약에 어빌리티 내에서 키 감지가 필요한 경우라면 설정해줘야 함
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InputID = INDEX_NONE;

	// 어빌리티 부여와 동시에 활성화 시킬지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bActivate = false;
};

UCLASS()
class TESTGAME_API UMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;

public:
	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent) const;
	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Filter) const;
	void ClearAbilities(UAbilitySystemComponent* AbilitySystemComponent) const;
	void ConverToMap(TMap<FGameplayTag, TSubclassOf<UGameplayAbility>>& TagToAbilityClassMap) const;
	UFUNCTION(BlueprintPure)
	FMAbilityBindInfo GetBindInfo(FGameplayTag InTag);
	bool HasAbilityTag(FGameplayTag InTag) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMAbilityBindInfo> Abilities;
	TMap<FGameplayTag, FMAbilityBindInfo> AbilityMap;
};

UCLASS()
class TESTGAME_API UGameplayAbility_MoveToMouse : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_MoveToMouse();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UFUNCTION()
	void MoveToMouse(FGameplayEventData Payload);
};

UCLASS()
class TESTGAME_API UGameplayAbility_CharacterBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CharacterBase();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

public:
	virtual FGameplayEffectSpecHandle MakeOutgoingGameplayEffectSpecWithIndex(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 InEffectIndex, float Level = 1.f) const;
	// MGameplayEffectContext를 업데이트 하려면 여기서 해야함
	virtual void UpdateEffectContext(FMGameplayEffectContextHandle& InEffectContextHandle) {}

protected:
	UMAbilitySystemComponent* GetMAbilitySystem();

public:
	UFUNCTION(BlueprintPure)
	AMCharacter* GetCharacter();
protected:
	FVector GetCharacterLocation(bool bIncludeCapsuleHeight);
	FRotator GetCharacterRotation();
protected:
	TWeakObjectPtr<AMCharacter> Character = nullptr;

public:
	// 어빌리티가 스폰한 액터는 이 함수를 호출해줘야 함
	UFUNCTION(BlueprintCallable)
	virtual void InitAbilitySpawnedActor(AActor* InActor);
	virtual void ApplyEffect(AActor* InEffectCauser, AActor* InTarget);
	// GE 적용 로직은 이 함수를 오버라이딩
	virtual void ApplyEffectToTarget(AActor* InEffectCauser, AActor* InTarget, const FGameplayAbilityTargetDataHandle& InTargetDataHandle);

protected:
	// 어빌리티 내에서 직접 대미지를 줄 경우 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect_Damage> DamageEffectClass = nullptr;

public:
	virtual int32 GetEffectIndex() const { return INDEX_NONE; }

/* 파라미터 */
public:
	UFUNCTION(BlueprintNativeEvent)
	void UpdateDynamicParams(AActor* OtherActor);
public:
	UFUNCTION(BlueprintCallable)
	void AddParam(const FGameplayTag& InTag, float InValue);
	UFUNCTION(BlueprintCallable)
	void AddParams(const TMap<FGameplayTag, float>& InParams);
	UFUNCTION(BlueprintCallable)
	void SetParams(const TMap<FGameplayTag, float>& InParams);
	UFUNCTION(BlueprintPure)
	float GetParam(FGameplayTag InTag);
	UFUNCTION(BlueprintPure)
	float GetParamUsingName(FName InName);
	UFUNCTION(BlueprintPure)
	const TMap<FGameplayTag, float>& GetParams() { return MapParamToValue; }
	UFUNCTION()
	void OnRep_Params();
	UFUNCTION(BlueprintCallable)
	void SetDynamicParam(FGameplayTag InTag, float InValue);
protected:
	void SerializeAttributeMap(FArchive& Archive);
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Params)
	TArray<uint8> SerializedParamsMap;
	// 스태틱 파라미터. SerailizedParam으로 변환해서 복제됨
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, float> MapParamToValue;
	// 다이나믹 파라미터.
	TMap<FGameplayTag, float> DynamicParamToValue;
};

UCLASS()
class TESTGAME_API UGameplayAbility_WeaponBase : public UGameplayAbility_CharacterBase
{
	GENERATED_BODY()

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	virtual void UpdateWeapon(AActor* Old, AActor* New);
protected:
	FDelegateHandle WeaponDamageDelegateHandle;

//	void ClearCachedData();
//protected:
//	bool bClearCacheIfEnd = true;
	
protected:
	TWeakObjectPtr<AWeapon> Weapon = nullptr;
	FDelegateHandle WeaponChangedDelegateHandle;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Combo : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Combo();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_BasicAttackStop : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_BasicAttackStop();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);
};

UCLASS()
class TESTGAME_API UGameplayAbility_SpinalReflex : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_SpinalReflex();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_CollideDamage : public UGameplayAbility_CharacterBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_CollideDamage();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnCollide(AActor* OverlappedActor, AActor* OtherActor);

public:
	UFUNCTION(BlueprintCallable)
	void SetDamage(float InDamage) { Damage = InDamage; }
protected:
	float Damage = 0.f;
	FGameplayTag CueTag = FGameplayTag::EmptyTag;
};

UCLASS()
class TESTGAME_API UGameplayAbility_KnockBack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_KnockBack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void KnockBack(AActor* OverlappedActor, AActor* OtherActor);
protected:
	UPROPERTY(BlueprintReadWrite)
	float Radius = 100.f;
	UPROPERTY(BlueprintReadWrite)
	float Strength = 0.f;
};

// DEPRECATED
UCLASS()
class TESTGAME_API UGameplayAbility_Move : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Move();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetForward(float InFoward) { Foward = InFoward; }
	UFUNCTION(BlueprintCallable)
	void SetStrafe(float InStrafe) { Strafe = InStrafe; }
protected:
	float Foward = 0.f;
	float Strafe = 0.f;
	FTimerHandle ReleaseHandle;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Move_KeepBasicAttack : public UGameplayAbility_Move
{
	GENERATED_BODY()

public:
	UGameplayAbility_Move_KeepBasicAttack();
};

UCLASS()
class TESTGAME_API UGameplayAbility_CameraShake : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CameraShake();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;
	TArray<TWeakObjectPtr<APlayerController>> TargetPlayers;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Reload : public UGameplayAbility_WeaponBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Reload();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UFUNCTION()
	void OnMontageFinished();
};

// 피격 시 주변 공격
UCLASS()
class TESTGAME_API UGameplayAbility_CounterAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_CounterAttack();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> CounterAttackClass = nullptr;
};

UCLASS()
class TESTGAME_API UGameplayAbility_DamageToOne : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_DamageToOne();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class TESTGAME_API UGameplayAbility_Start : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Start();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	class UAnimMontage* GetStartAnim();
};

UCLASS()
class TESTGAME_API UGameplayAbility_LevelUp : public UGameplayAbility_CharacterBase
{
	GENERATED_BODY()

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	void UpdateMesh(int32 InLevel);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int32, FSoftObjectPath> LevelToMesh;
};