#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DamageComponent.generated.h"

class UCapsuleComponent;
class UGameplayEffect;
class UGameplayEffect_Damage;

DECLARE_EVENT_TwoParams(UMDamageComponent, FOnDamagEvent, AActor*, AActor*)

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class TESTGAME_API UMDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable)
	bool GiveDamage(AActor* OtherActor);
	UFUNCTION()
	void Overlap(AActor* OverlappedActor, AActor* OtherActor);
	virtual void React(AActor* InActor);
	bool IsReactable(AActor* InActor);
protected:
	TArray<TWeakObjectPtr<AActor>> IgnoreActors;
	UPROPERTY(EditDefaultsOnly)
	bool bApplyDamageOnOverlap = true;

/* 대미지 */
public:
	FOnDamagEvent& GetOnDamageEvent() { return OnDamageEvent; }
protected:
	FOnDamagEvent OnDamageEvent;

public:
	UFUNCTION()
	void UpdateHold();
	UFUNCTION(BlueprintCallable)
	void Reset();
protected:
	// 대미지 입힐 수 있는지 여부. ex) 근접 무기는 활성화 전에는 대미지 적용되지 않게 만듬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bDamagable = false;
	UPROPERTY()
	TMap<AActor*, int32> DamageCountMap;
	UPROPERTY()
	TMap<AActor*, float> DamageTimeMap;
	UPROPERTY()
	TSet<AActor*> DamageHold;
	UPROPERTY(EditDefaultsOnly)
	float Period = 0.1f;
	UPROPERTY(EditDefaultsOnly)
	int32 DamageApplyMaxCount = 1;
	FTimerHandle DamageHoldUpdateTimerHandle;
	UPROPERTY(EditDefaultsOnly)
	float HoldDistance = 20.f;
	TWeakObjectPtr<UCapsuleComponent> OwnerCapsule = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void ChangeDamageEffectClass(TSubclassOf<UGameplayEffect_Damage> InDamageEffectClass) { DamageEffectClass = InDamageEffectClass; }
protected:
	// 대미지 처리를 위한 GE. 대미지 계산식, 플로팅 UI 큐가 세팅된 GE.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect_Damage> DamageEffectClass = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void AddGameplayEffect(TSubclassOf<UGameplayEffect> InEffectClass);
protected:
	// 대상에게 적용할 GE. 슬로우 등
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> Effects;


public:
	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* InTarget) { Target = InTarget; }
protected:
	// 대미지를 입힐 대상. 설정된 경우 다른 액터들은 무시됨.
	TWeakObjectPtr<AActor> Target = nullptr;

protected:
	// 대미지 유발자 
	TWeakObjectPtr<APawn> DamageInstigator = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void SetDamageParams(const TMap<FGameplayTag, float>& InParams);
protected:
	// 대미지 계산식에 사용되는 파라미터. ex) 공격력 비율, 추가대미지
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, float> DamageParams;

protected:
	// 서브 GE 파라미터. Duration, Value 설정
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, float> EffectParams;
};
