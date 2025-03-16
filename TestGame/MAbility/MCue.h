#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayCueNotify_Actor.h"
#include "MCue.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UMaterialParameterCollection;

// 적절한 곳으로 옮기기
USTRUCT(BlueprintType)
struct TESTGAME_API FGameplayTagTextTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override
	{
		Super::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
		GameplayTag = FGameplayTag::RequestGameplayTag(InRowName);
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GameplayTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Text;
};

USTRUCT(BlueprintType)
struct TESTGAME_API FGameplayCueUIData
{
	GENERATED_BODY()

public:
	// 이펙트의 타겟
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	AActor* Target = nullptr;

	// 이펙트 스펙
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayEffectSpec GameplayEffectSpec;

	// 활성화 중인 이펙트 핸들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
	
	// 큐 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayCueParameters GameplayCueParams;

	// 이펙트가 수정하는 값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Value = 0.f;
};

// 적절한 곳으로 옮기기
UINTERFACE(BlueprintType)
class TESTGAME_API UObjectByGameplayCue : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IObjectByGameplayCue
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void InitByGameplayCue(const FGameplayCueUIData& InEffectStatusData);
};


UCLASS()
class TESTGAME_API UGameplayCue_FloatMessage : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class TESTGAME_API UGameplayCue_StatusEffect : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	mutable FTimerHandle TimerHandle;
	mutable TWeakObjectPtr<UUserWidget> Widget = nullptr;
	UPROPERTY(EditDefaultsOnly)
	bool bOnlyAutonomous = false;
};

UCLASS()
class TESTGAME_API AGameplayCue_CounterAttack : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGameplayCue_CounterAttack();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraComponent* NiagaraComponent = nullptr;

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
};

// 레거시 이펙트인 Cascade 전용. NiagaraSystem을 사용한다면 GameplayCueNotify_Burst를 사용!
UCLASS()
class TESTGAME_API UGameplayCue_CascadeParticle : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

public:
	UFUNCTION()
	void RemoveTarget(AActor* InActor, EEndPlayReason::Type InEndPlayReason);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UFXSystemAsset* Particle = nullptr;
	mutable TMap<TWeakObjectPtr<AActor>, class UFXSystemComponent*> Test;
	mutable TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> MapSoundHelper;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> SoundHelperClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SocketName = NAME_None;
};

UCLASS()
class TESTGAME_API UGameplayCue_SkillCoolDown : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class TESTGAME_API UGamepalyCue_Freeze : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};