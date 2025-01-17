// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MGameInstance.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MAttribute/MAttribute.h"

DECLARE_LOG_CATEGORY_CLASS(LogWeapon, Log, Log);

AWeapon::AWeapon()
{
	ActionComponent = CreateDefaultSubobject<UMActionComponent>(TEXT("ActionComponent"));

	//AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	//bReplicates = true;
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (const FWeaponData* WeaponData = GetWeaponData())
	{
		TArray<UShapeComponent*> ShapeComponents;
		GetComponents(ShapeComponents);

		// 무기의 메시 설정과 쉐이프를 메시에 어태치
		if (USkeletalMeshComponent* SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>())
		{
			SkeletalMeshComponent->SetSkeletalMesh(WeaponData->Mesh);
			for (UShapeComponent* ShapeComponent : ShapeComponents)
			{
				ShapeComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false), TEXT("Root"));
			}
		}

		switch (WeaponData->WeaponType)
		{
			case EWeaponType::Sword:
			{
				for (UShapeComponent* ShapeComponent : ShapeComponents)
				{
					ShapeComponent->SetGenerateOverlapEvents(true);
				}
			}
			break;
			default:
			{
				for (UShapeComponent* ShapeComponent : ShapeComponents)
				{
					ShapeComponent->SetGenerateOverlapEvents(false);
				}
			}
			break;
		}
	}
}

//void AWeapon::NotifyActorBeginOverlap(AActor* OtherActor)
//{
//	Super::NotifyActorBeginOverlap(OtherActor);
//
//	if (IsReactable(OtherActor) && HasAuthority() && IsValid(Owner))
//	{
//		if (UAbilitySystemComponent* AbilitySystemComponent = Owner->GetComponentByClass<UAbilitySystemComponent>())
//		{
//			FGameplayEffectSpecHandle GameplayEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UGameplayEffect_Damage::StaticClass(), -1, AbilitySystemComponent->MakeEffectContext());
//			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*GameplayEffectSpecHandle.Data.Get(), OtherActor->GetComponentByClass<UAbilitySystemComponent>());
//
//			// 넉백 처리, 임시로 한거고 추후에 넉백저항 등이 들어가면 어트리뷰트에 기반해야 할 듯?
//			FVector Offset = FVector::ZeroVector;
//			if (UCapsuleComponent* CapsuleComponent = Owner->GetComponentByClass<UCapsuleComponent>())
//			{
//				Offset.Z = CapsuleComponent->GetScaledCapsuleHalfHeight();
//			}
//			if (UCharacterMovementComponent* MovementComponent = OtherActor->GetComponentByClass<UCharacterMovementComponent>())
//			{
//				MovementComponent->StopMovementImmediately();
//				MovementComponent->AddRadialImpulse(Owner->GetActorLocation() + Offset, 1000.f, 150000.f, ERadialImpulseFalloff::RIF_Linear, false);
//			}
//		}
//	}
//}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, WeaponIndex, COND_InitialOnly);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (const FWeaponData* WeaponData = GetWeaponData())
	{
		if (IsValid(ActionComponent))
		{
			ActionComponent->UpdateActionData(WeaponData->ActionData);
		}
	}

	//if (const FWeaponData* WeaponData = GetWeaponData())
	//{
	//	AbilitySystemComponent->InitStats(GetAttributeSet(), WeaponData->Attributes);
	//}

	SetActorEnableCollision(false);
}

TSubclassOf<UAttributeSet> AWeapon::GetAttributeSet()
{
	return UMWeaponAttributeSet::StaticClass();
}

void AWeapon::SetEquipActor(AActor* EquipActor)
{
	if (IsValid(EquipActor))
	{
		if (const FWeaponData* WeaponData = GetWeaponData())
		{
			FString SocketName;
			switch (WeaponData->WeaponType)
			{
			default:
			case EWeaponType::Sword:
			{
				SocketName = TEXT("hand_rSocket");
			}
			break;
			case EWeaponType::Gun:
			{
				SocketName = TEXT("hand_rSocket_FPS");
			}
			break;
			}

			AttachToActor(EquipActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), *SocketName);
		}
	}

	SetOwner(EquipActor);

	if (AMCharacter* Character = Cast<AMCharacter>(EquipActor))
	{
		EquipChangedHandle = Character->OnWeaponChangedEvent.AddUObject(this, &AWeapon::OnEquipmentChanged);
		Character->EquipItem(this);
	}
}

void AWeapon::OnEquipmentChanged(AActor* OldWeapon, AActor* NewWeapon)
{
	AMCharacter* Character = Cast<AMCharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		return;
	}

	if (OldWeapon == this)
	{
		SetActorHiddenInGame(true);
		SetLifeSpan(0.1f);
		// 기존 무기의 어빌리티를 모두 버림 -> 버리고 끼면 어빌리티 쿨같은게 초기화 되있을 듯?
		if (const FWeaponData* WeaponData = GetWeaponData())
		{
			//OwningCharacter->RemoveAbilities(WeaponData->AbilitiesDataAsset);
		}

		if (EquipChangedHandle.IsValid())
		{
			Character->OnWeaponChangedEvent.Remove(EquipChangedHandle);
			EquipChangedHandle.Reset();
		}
	}

	if (NewWeapon == this)
	{
		// 새로운 무기의 어빌리티를 모두 가짐
		if (const FWeaponData* WeaponData = GetWeaponData())
		{
			Character->AddAbilities(WeaponData->AbilitiesDataAsset);
		}

		// 애니메이션 갱신
		if (UMActionComponent* CharacterActionComponent = Character->GetComponentByClass<UMActionComponent>())
		{
			CharacterActionComponent->UpdateAction(ActionComponent);
		}

		if (UAbilitySystemComponent* CharacterAbilityComponent = Character->GetComponentByClass<UAbilitySystemComponent>())
		{
			CharacterAbilityComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetWeaponScaleAttribute()).AddUObject(this, &AWeapon::ChangeWeaponScale);

			FOnAttributeChangeData ChangedData;
			ChangedData.Attribute = nullptr;
			ChangedData.OldValue = 0.f;
			ChangedData.NewValue = CharacterAbilityComponent->GetNumericAttribute(UMAttributeSet::GetWeaponScaleAttribute());
			ChangeWeaponScale(ChangedData);
		}
	}
}

void AWeapon::ChangeWeaponScale(const FOnAttributeChangeData& AttributeChangeData)
{
	SetActorScale3D(FVector(AttributeChangeData.NewValue));
}

void AWeapon::OnRep_WeaponIndex()
{
	if (HasAuthority() == false)
	{
		if (const FWeaponData* WeaponData = GetWeaponData())
		{
			if (USkeletalMeshComponent* SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>())
			{
				SkeletalMeshComponent->SetSkeletalMesh(WeaponData->Mesh);
			}
		}
	}
}

void AWeapon::SetAttackMode(FGameplayTag InAttackModeTag)
{
	if (AttackMode != InAttackModeTag)
	{
		AttackMode = InAttackModeTag;
		Combo = INDEX_NONE;
	}
}

void AWeapon::SetWeaponIndex(int32 InIndex)
{
	WeaponIndex = InIndex;
}

const FWeaponData* AWeapon::GetWeaponData() const
{
	if (IsValid(WeaponDataTable) && WeaponIndex != INDEX_NONE)
	{
		return WeaponDataTable->FindRow<FWeaponData>(*FString::FromInt(WeaponIndex), TEXT("WeaponDataTable"));
	}

	return nullptr;
}

const FGameItemTableRow* AWeapon::GetItemTableRow() const
{
	if (UMGameInstance* GameInstace = GetGameInstance<UMGameInstance>())
	{
		return GameInstace->GetItemTableRow(WeaponIndex);
	}

	return nullptr;
}

bool AWeapon::GetMuzzleTransform(FTransform& OutTransform)
{
	if (USkeletalMeshComponent* WeaponMesh = GetComponentByClass<USkeletalMeshComponent>())
	{
		OutTransform = WeaponMesh->GetSocketTransform(TEXT("Muzzle"));
		return true;
	}

	return false;
}

void AWeapon::NextCombo()
{
	const FWeaponData* WeaponData = GetWeaponData();
	AMCharacter* Character = Cast<AMCharacter>(GetOwner());
	if (WeaponData == nullptr || IsValid(Character) == false)
	{
		return;
	}

	float OwnerAttackSpeed = 1.f;
	if (UAbilitySystemComponent* CharacterAbilityComponent = Character->GetAbilitySystemComponent())
	{
		bool bFound = false;
		CharacterAbilityComponent->GetGameplayAttributeValue(UMAttributeSet::GetAttackSpeedAttribute(), bFound);
		if (bFound == false)
		{
			UE_LOG(LogWeapon, Warning, TEXT("Need attribute AttackSpeed. Failed to attack"));
			return;
		}
	}

	// 임시작업, 공격속도 어트리뷰트를 가져오도록 변경해야 함
	// 공격속도가 빨라진다 = AttackSpeed 증가를 의미하는데 이게 맞는건가?
	if (WeaponData->AttackSpeed > 0.f)
	{
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AWeapon::OnAttackCoolDownFinished, WeaponData->AttackSpeed, false);
	}

	if (WeaponData->WeaponType == EWeaponType::Sword)
	{
		if (AttackMode == FGameplayTag::RequestGameplayTag("Action.Attack.Light") || AttackMode == FGameplayTag::RequestGameplayTag("Action.Attack.ChargeLight"))
		{
			Character->LookMouse(-1.f);
		}
		else if (AttackMode == FGameplayTag::RequestGameplayTag("Action.Attack.DashLight"))
		{

		}
	}

	bCoolDown = true;
	Combo = IsComboableWeapon() && IsComboable() ? Combo + 1 : 0;

	OnComboChangedEvent.Broadcast(Combo);

	UE_LOG(LogTemp, Warning, TEXT("Combo:%d"), Combo);
}

void AWeapon::ResetCombo()
{
	Combo = INDEX_NONE;
	OnComboChangedEvent.Broadcast(Combo);
}

void AWeapon::OnAttackCoolDownFinished()
{
	bCoolDown = false;
}

bool AWeapon::IsAttackable() const
{
	const FWeaponData* WeaponData = GetWeaponData();

	if (WeaponData == nullptr)
	{
		return false;
	}


	if (WeaponData->WeaponType == EWeaponType::Gun)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetComponentByClass<UAbilitySystemComponent>())
		{
			return static_cast<int>(AbilitySystemComponent->GetNumericAttribute(UMWeaponAttributeSet::GetAmmoAttribute())) > 0;
		}
	}

	return true;
}

bool AWeapon::IsCoolDown() const
{
	return bCoolDown;
}

bool AWeapon::IsAttacking() const
{
	return true;
}

void AWeapon::Charge()
{
	if (IsCharged() == false)
	{
		//bCharge = true;
		//if (UWorld* World = GetWorld())
		//{
		//	ChargeValue += World->GetDeltaSeconds();
		//	ChargeValue = FMath::Min(ChargeValue, 1.f);
		//}

		ChargeValue = 1.f;

		if (IsCharged())
		{
			OnChargeChangedEvent.Broadcast(true);
		}
	}
}

void AWeapon::UnCharge()
{
	if (IsCharged())
	{
		ChargeValue = 0.f;
		OnChargeChangedEvent.Broadcast(false);
	}
}

bool AWeapon::IsComboableWeapon() const
{
	if (const FWeaponData* WeaponData = GetWeaponData())
	{
		return WeaponData->Combo != INDEX_NONE;
	}

	return false;
}

bool AWeapon::IsComboable() const
{
	if (const FWeaponData* WeaponData = GetWeaponData())
	{
		return Combo + 1 < WeaponData->Combo;
	}

	return false;
}

ADelegator::ADelegator()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	bReplicates = true;	
}

void ADelegator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	AWeapon* Weapon = Cast<AWeapon>(GetOwner());
	if (Weapon == nullptr)
	{
		if (AMCharacter* Character = Cast<AMCharacter>(GetOwner()))
		{
			Weapon = Character->GetEquipItem<AWeapon>();
		}
	}
}
