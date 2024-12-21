// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MCharacter/Component/StateMachineComponent.h"

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineStats.h"
#include "TestGame/Interface/InteractInterface.h"
#include "AbilitySystemInterface.h"

#include "MCharacter.generated.h"

class UMBattleComponent;
class UMAttributeSet;
class UMAbilityDataAsset;
class UStateComponent;
class UNiagaraComponent;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDieDelegate);
DECLARE_EVENT_TwoParams(AMCharacter, FOnWeaponChangedEvent, AActor*, AActor*);

UCLASS()
class TESTGAME_API AMCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMCharacter();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMBattleComponent* BattleComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStateComponent* StateComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraComponent* NiagaraComponent = nullptr;
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
	void OnAbilityInputPressed(int32 InInputID);
	void OnAbilityInputReleased(int32 InInputID);
	void AddAbilities(UMAbilityDataAsset* AbilityDataAsset);
	void RemoveAbilities(UMAbilityDataAsset* AbilityDataAsset);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMAbilityDataAsset* AbilitySetData;
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AblitiyHandles;

public:
	void ChargeInputPressed();
protected:
// 입력과 어빌리티 바인딩 테스트
	UPROPERTY(EditAnywhere)
	class UInputAction* InputAction = nullptr;
	UPROPERTY(EditAnywhere)
	class UInputAction* InputAction2 = nullptr;
	UPROPERTY(EditAnywhere)
	class UInputAction* InputAction3 = nullptr;
	bool bChargeInput = false;

	//UFUNCTION(BlueprintPure)
	//FGameplayAbilitySpecHandle GetAbiltiyTypeHandle(EAbilityType AbilityType);

// GAS Attribute
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealthbarWidget(float OldValue, float NewValue);
public:
	virtual void OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnDamaged(AActor* DamageInstigator);
protected:
	UPROPERTY(BlueprintReadOnly)
	UMAttributeSet* AttributeSet;

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

// 장비
	enum class EEquipType {
		None, Weapon, Max
	};
public:
	FOnWeaponChangedEvent OnWeaponChangedEvent;
	FTimerDelegate TestDelegate;
public:
	UFUNCTION(BlueprintPure)
	bool GetWeaponMuzzleTransform(FTransform& OutTransform);	// 총알 나가는 소켓 위치. 이름은 임시로
	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	UFUNCTION(BlueprintCallable)
	void EquipItem(AActor* InItem);
	UFUNCTION(BlueprintPure)
	bool GetWeaponData(FWeaponData& OutWeaponData);
	UFUNCTION()
	void OnRep_Weapon(AActor* OldWeapon);
	template <class T>
	T* GetEquipItem()
	{
		return Cast<T>(Item);
	}
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Weapon)
	AActor* Item = nullptr;
	AActor* WeaponCached = nullptr;
	TArray<FGameplayAbilitySpecHandle> WeaponAbilitySpecHandles;

// 공격
public:
	void Aim();
	UFUNCTION(BlueprintCallable)
	void BasicAttack();
	UFUNCTION(BlueprintCallable)
	void FinishBasicAttack();
	void ChargeLightAttack();
	void LightChargeAttack();
	void FinishCharge();

// 타겟팅 -> 컴포넌트로 뺴기
public:
	void LookMouse(float TurnSpeed = 1.f);
	UFUNCTION(Server, Unreliable)
	void Server_SetTargetAngle(float InTargetAngle, bool bInstantly);
	void SetRotateToTargetAngle(bool bNewValue);
	UFUNCTION(Server, Unreliable)
	void Server_SetRotateToTargetAngle(bool bNewValue);
	//float GetTargetAngle() { return TargetAngle; }
	UFUNCTION()
	void OnRep_TargetAngle();
	UPROPERTY(ReplicatedUsing = OnRep_TargetAngle)
	float TargetAngle = 0.f;
	//UPROPERTY(Replicated)
	//bool bRotateToTargetAngle = false;
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


// 매터리얼 헬퍼
	FTimerHandle Handle;
	float Opacity = 0.f;
	int32 ActionNumber = 0;
	void SetOpacity(float InOpacity)
	{
		SetMaterialParam([InOpacity](UMaterialInstanceDynamic* DynamicMaterialInstance) {
			DynamicMaterialInstance->SetScalarParameterValue("Opacity", InOpacity);
		});
	}

	void SetMaterialParam(TFunction<void(UMaterialInstanceDynamic*)> Func)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			for (int MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); ++MaterialIndex)
			{
				if (UMaterialInstanceDynamic* DynamicMaterialInstance = MeshComp->CreateDynamicMaterialInstance(MaterialIndex))
				{
					Func(DynamicMaterialInstance);
				}
			}
		}
	}

public:
// 리더보드
	UFUNCTION(BlueprintCallable)
	void LeaderboardTest();
	FOnlineLeaderboardReadPtr ReadObject;
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;
	void PrintLeaderboard(bool b);
};