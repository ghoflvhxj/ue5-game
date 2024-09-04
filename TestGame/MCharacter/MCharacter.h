// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MCharacter/Component/StateMachineComponent.h"
//#include "GameplayEffectTypes.h"

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineStats.h"
//GameAbilitySystem
//#include "GameplayAbilities/Public/GameplayEffectTypes.h"

#include "TestGame/Interface/InteractInterface.h"
#include "AbilitySystemInterface.h"

#include "MCharacter.generated.h"

class UMBattleComponent;
class UMAttributeSet;
class UMAbilityDataAsset;
class UStateComponent;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDieDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedDelegate, float, OldValue, float, NewValue, float, MaxValue);

UCLASS()
class TESTGAME_API AMCharacter : public ACharacter, public IAbilitySystemInterface
{
	/*--------------------------------------------------------------
	캐릭터는 상태를 관리합니다.
	공존할 수 없는 상태들의 집합으로 카테고리가 만들 수 있습니다. ex) 앉음, 일어서있음 상태는 공존할 수 없기 때문에 포즈 카테고리로 묶입니다.
	
	현재 제공하는 것은 생명, 베이스포즈, 확장포즈, 액션 3개의 카테고리입니다.
	ex) 
	1. 살아있는 캐릭터가(생명상태), 일어선 상태에서(포즈상태), 가만히 있으면서(이동상태), 공격한다(액션상태)
	2. 살아있는 캐릭터가(생명상태) 일어선 상태에서(포즈상태), 이동하면서(이동상태), 공격한다(액션상태)

	--------------------------------------------------------------*/
	GENERATED_BODY()

public:
	AMCharacter();

// 언리얼 인터페이스
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

// GAS Ability
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMAbilityDataAsset* AbilitySetData;

	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AblitiyHandles;

	//UFUNCTION(BlueprintPure)
	//FGameplayAbilitySpecHandle GetAbiltiyTypeHandle(EAbilityType AbilityType);

// GAS Attribute
public:
	UFUNCTION(BlueprintCallable)
	void EffectTest();
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFloaterWidget(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealthbarWidget();
public:
	virtual void OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
protected:
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FOnHealthChangedDelegate OnHealthChangedDelegate;
protected:
	UPROPERTY(BlueprintReadOnly)
	UMAttributeSet* AttributeSet;

// 컴포넌트
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent* SearchComponent;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMBattleComponent* BattleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStateComponent* StateComponent;
public:
	UPrimitiveComponent* GetComponentForAttackSearch();
private:
	template <class T>
	void CompoentLogic(TFunction<void(T* Component)> Logic)
	{
		T* Component = GetComponentByClass<T>();
		if (IsValid(Component))
		{
			Logic(Component);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Component Logic Failed"));
		}
	}

// 기능 테스트
public:
	UFUNCTION(BlueprintCallable)
	void TestFunction1();
	UFUNCTION(BlueprintCallable)
	void TestFunction2();
	UFUNCTION(BlueprintCallable)
	void TestFunction3();

// 상태 델레게이트
public:
	void AddVitalityChangedDelegate(UObject* Object, const TFunction<void(uint8 OldValue, uint8 NewValue)> Function);
// 상태
public:
	template <typename T>
	bool IsState(T StateValue)
	{
		if (UStateComponent* StateMachineComponent = GetComponentByClass<UStateComponent>())
		{
			T CurrentState = (T)0;
			if (StateMachineComponent->GetState<T>(CurrentState))
			{
				if (StateValue == CurrentState)
				{
					return true;
				}
			}
		}

		return false;
	}
	UFUNCTION(BlueprintCallable)
	bool IsDead();

// 팀
public:
	UFUNCTION(BlueprintCallable)
	bool IsSameTeam(AActor* OtherCharacter) const;

// 무기
	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AWeapon* InWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipWeapon(AWeapon* InWeapon);
	UPROPERTY(Replicated)
	TWeakObjectPtr<AWeapon> Weapon = nullptr;

// 공격
	UFUNCTION(BlueprintCallable)
	bool IsAttackable();

public:
	void UpdateTargetAngle();
	UFUNCTION(Server, Unreliable)
	void Server_UpdateTargetAngle(float InTargetAngle);
	void SetRotateToTargetAngle(bool bNewValue);
	UFUNCTION(Server, Unreliable)
	void Server_SetRotateToTargetAngle(bool bNewValue);
	float GetTargetAngle() { return TargetAngle; }
	UFUNCTION()
	void OnRep_TargetAngle();
	UPROPERTY(ReplicatedUsing = OnRep_TargetAngle)
	float TargetAngle = 0.f;
	UPROPERTY(Replicated)
	bool bRotateToTargetAngle = false;
private:
	void RotateToTargetAngle();

// 이동
public:
	UFUNCTION(BlueprintCallable)
	void MoveToLocation();

// 상호작용
public:
	bool IsInteractableActor(AActor* OtherActor);
private:
	IInteractInterface* GetInteractInterface(AActor* Actor);
private:
	TArray<TWeakObjectPtr<AActor>> InteractTargets;

public:
// 리더보드
	UFUNCTION(BlueprintCallable)
	void LeaderboardTest();
	FOnlineLeaderboardReadPtr ReadObject;
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;
	void PrintLeaderboard(bool b);
};