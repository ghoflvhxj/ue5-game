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
DECLARE_EVENT_TwoParams(AMCharacter, FOnWeaponChangedEvent, AWeapon*, AWeapon*);

UCLASS()
class TESTGAME_API AMCharacter : public ACharacter, public IAbilitySystemInterface
{
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
	virtual USceneComponent* GetDefaultAttachComponent() const override { return GetMesh(); }

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
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealthbarWidget(float OldValue, float NewValue);
public:
	virtual void OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
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
public:
	FOnWeaponChangedEvent OnWeaponChangedEvent;
	FTimerDelegate TestDelegate;
public:
	UFUNCTION(BlueprintPure)
	bool GetWeaponMuzzleTransform(FTransform& OutTransform);	// 총알 나가는 소켓 위치. 이름은 임시로
	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AWeapon* InWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ChangeWeapon(AWeapon* InWeapon);
	UFUNCTION()
	void OnRep_Weapon(AWeapon* OldWeapon);
	template <class T>
	T* GetWeapon()
	{
		return Cast<T>(Weapon);
	}
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Weapon)
	AWeapon* Weapon = nullptr;

// 공격
public:
	UFUNCTION(BlueprintCallable)
	void TryBasicAttack();
	void StartBasicAttack();
	void FinishBasicAttack();
	UFUNCTION(BlueprintCallable)
	bool IsAttackable();

// 타겟팅 -> 컴포넌트로 뺴기
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