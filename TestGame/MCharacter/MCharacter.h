// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
#include "AbilitySystemComponent.h"
#include "OnlineStats.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"

#include "TestGame/Interface/InteractInterface.h"

#include "MCharacter.generated.h"

class UMAbilitySystemComponent;
class UGameplayAbility;

class UMTeamComponent;
class UMAttributeSet;
class UMAbilityDataAsset;
class UNiagaraComponent;
class UMActionComponent;
class UMInteractorComponent;
class UMInventoryComponent;
class AWeapon;

DECLARE_EVENT(AMCharacter, FOnDeadEvent);
DECLARE_EVENT_TwoParams(AMCharacter, FOnWeaponChangedEvent, AActor*, AActor*);
DECLARE_EVENT_OneParam(AMCharacter, FOnItemUsedEvent, int32);

UENUM(BlueprintType)
enum class EAbilityInputType : uint8
{
	NONE,
	Attack,
	Combo,
};

UCLASS()
class TESTGAME_API AMCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMCharacter();
public:
	UMTeamComponent* GetTeamComponent() const { return TeamComponent; }
	UMActionComponent* GetActionComponent() const { return ActionComponent; }
	UFUNCTION(BlueprintPure)
	UMInventoryComponent* GetInventoryComponent() const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMAbilitySystemComponent* AbilitySystemComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMTeamComponent* TeamComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraComponent* NiagaraComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMActionComponent* ActionComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMInventoryComponent* InventoryComponent = nullptr;
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

public:
	UFUNCTION(BlueprintPure)
	virtual int32 GetCharacterIndex() { return INDEX_NONE; }

/* AbilitySystem 관련 */
public:
	void OnAbilityInputPressed(int32 InInputID);
	void OnAbilityInputReleased(int32 InInputID);
	void AddAbilities(UMAbilityDataAsset* AbilityDataAsset);
	void RemoveAbilities(UMAbilityDataAsset* AbilityDataAsset);
	UFUNCTION(BlueprintPure)
	FGameplayAbilitySpecHandle GetAbilitySpecHandle(FGameplayTag InTag);
	UFUNCTION(BlueprintPure)
	UGameplayAbility* GetAbility(FGameplayTag InTag);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMAbilityDataAsset* AbilitySetData = nullptr;

public:
	virtual void OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	virtual void OnDamaged(AActor* DamageInstigator);
public:
	float GetAttribute(const FName& InAttributeName);
protected:
	UPROPERTY(BlueprintReadOnly)
	UMAttributeSet* AttributeSet = nullptr;

public:
	void MakeEffectCue(const FGameplayTag& InTag);
	void MakeEffectCue(int32 InEffectIndex);
	virtual int32 GetEffectIndex(const FGameplayTag& InTag) const { return INDEX_NONE; }

/* 인풋 관련 */
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


/* 캐릭터 상태 */
public:
	UFUNCTION(BlueprintPure)
	bool IsDead();
public:
	FOnDeadEvent& GetOnDeadEvent() { return OnDeadEvent; }
protected:
	FOnDeadEvent OnDeadEvent;

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
	virtual UPrimitiveComponent* GetWeaponCollision();
	void SetWeaponActivation(bool bActivate, float InTime);
	UFUNCTION(BlueprintNativeEvent)
	void SetWeaponCollisionEnable(bool bInEnable);
	UFUNCTION(BlueprintPure)
	bool GetWeaponMuzzleTransform(FTransform& OutTransform);	// 총알 나가는 소켓 위치. 이름은 임시로
	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	UFUNCTION(BlueprintCallable)
	void EquipItem(int32 InItemIndex);
	FName GetEquipSocketName();
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
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FName> EquipTypeToSocektName;
	UPROPERTY(ReplicatedUsing = OnRep_Weapon)
	AActor* Item = nullptr;
	AActor* WeaponCached = nullptr;
	TArray<FGameplayAbilitySpecHandle> WeaponAbilitySpecHandles;
	FDelegateHandle ComboReserveHandle;

/* 아이템 */
public:
	void AddItem(int32 InIndex, int32 InNum);
	void UseItem(int32 InIndex);
	FOnItemUsedEvent& GetOnItemUsedEvent() { return OnItemUsedEvent; }
protected:
	FOnItemUsedEvent OnItemUsedEvent;

// 공격
public:
	void Aim();
	UFUNCTION(BlueprintCallable)
	virtual void BasicAttack();
	UFUNCTION(BlueprintCallable)
	void FinishBasicAttack();
	void ChargeLightAttack();
	void LightChargeAttack();
	void FinishCharge();

// 타겟팅 -> 컴포넌트로 뺴기
public:
	UFUNCTION(BlueprintCallable)
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

/* 넉백, 기절 등의 상태 */
public:
	UFUNCTION(BlueprintCallable)
	void KnockBack(const FVector& InLocation, float Radius, float Strength);

// 이동
public:
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
	UFUNCTION(BlueprintCallable)
	void MoveToLocation();
	void SetMoving(bool bInMoving);
	UFUNCTION(Server, Reliable)
	void Server_SetMoving(bool bInMoving);
protected:
	// 이동 중인지 여부. MoveAbility가 활성화 중이면 True임
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bMoving = false;

public:
	UFUNCTION(BlueprintCallable)
	virtual void Freeze(const FGameplayTag InTag, int32 InStack) {}

// 상호작용
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
	
// 액션
public:
	UFUNCTION(BlueprintCallable)
	void Ragdoll();
	void PlayStartAnim();
	UFUNCTION(BlueprintNativeEvent)
	void OnStartAnimFinished(UAnimMontage* Montage, bool bInterrupted);
public:
	UFUNCTION(Server, Reliable)
	void Server_HasBegunPlay();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartFinishedDelegate);
	UPROPERTY(BlueprintAssignable)
	FOnStartFinishedDelegate OnStartFinishedDelegate;

// 임시 이펙트 스택
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddEffectStack(FGameplayTag InTag);

public:
	UFUNCTION(Server, Reliable)
	void Server_AimMode(bool InAimMode);
	UFUNCTION(BlueprintCallable)
	void AimMode();
	UFUNCTION(BlueprintCallable)
	void MoveMode();
	UFUNCTION()
	void OnRep_AimMode();
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AimMode)
	bool bAimMode = false;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void FootStep(EPhysicalSurface InPhysicalSurface);
};