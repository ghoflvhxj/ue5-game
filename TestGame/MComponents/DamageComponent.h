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
	UMDamageComponent();

protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
public:
	UFUNCTION(BlueprintCallable)
	void TryGiveDamage(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION(BlueprintCallable)
	bool GiveDamage(AActor* OtherActor);
	UFUNCTION()
	void HoldTarget(AActor* OverlappedActor, AActor* OtherActor);
	virtual void React(AActor* InActor);
	bool IsReactable(AActor* InActor);
protected:
	TArray<TWeakObjectPtr<AActor>> IgnoreActors;
	// 오버랩을 이용해 대미지를 줄지 설정.
	UPROPERTY(EditDefaultsOnly)
	bool bApplyDamageOnOverlap = true;

/* 대미지 */
public:
	FOnDamagEvent& GetOnDamageEvent() { return OnDamageEvent; }
protected:
	FOnDamagEvent OnDamageEvent;

public:
	UFUNCTION(BlueprintCallable)
	void Reset();
	UFUNCTION(BlueprintCallable)
	void SetPeriod(float InValue);
protected:
	// 대미지 입힐 수 있는지 여부. ex) 근접 무기는 활성화 전에는 대미지 적용되지 않게 만듬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bDamagable = false;

	// 입힌 대미지 횟수를 저장
	UPROPERTY()
	TMap<AActor*, int32> MapTargetToDamageCount;

	// 마지막으로 입힌 대미지 시간을 저장
	UPROPERTY()
	TMap<AActor*, float> MapTargetToLastDamageTime;

	// 대미지 주기
	UPROPERTY(EditDefaultsOnly)
	float Period = 0.1f;

	// 대미지 최대 횟수로 0이면 무제한
	UPROPERTY(EditDefaultsOnly)
	int32 DamageApplyMaxCount = 1;

/* 멀티플레이 환경에서 대미지가 의도치 않게 여러 번 적용되는 경우을 방지하는 '홀드'기능 */ 
protected:
	// 홀드 거리. 이 거리 이상으로 벌어지지 않으면 홀드함.
	UPROPERTY(EditDefaultsOnly)
	float HoldDistance = 20.f;
	// 홀드 시간. 사용되지 않음
	UPROPERTY(EditDefaultsOnly)
	float HoldTime = 0.1f;
	// 홀드맵
	UPROPERTY()
	TMap<AActor*, float> MapTargetToDamageHold;

protected:
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
