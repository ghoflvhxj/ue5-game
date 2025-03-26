// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraComponent.h"

#include "MGameInstance.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MComponents/DamageComponent.h"
#include "TestGame/MAttribute/MAttribute.h"

DECLARE_LOG_CATEGORY_CLASS(LogWeapon, Log, Log);

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	ActionComponent = CreateDefaultSubobject<UMActionComponent>(TEXT("ActionComponent"));

	DamageComponent = CreateDefaultSubobject<UMDamageComponent>(TEXT("DamageComponent"));

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

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UNiagaraComponent* TrailComponent = GetComponentByClass<UNiagaraComponent>())
	{
		TrailComponent->SetFloatParameter(TEXT("TrailWidth"), GetTrailWidth());
	}
}

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

	SetActorEnableCollision(false);
}

void AWeapon::Activate(float InTime)
{
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj WeaponActivate"));

	if (IsValid(DamageComponent))
	{
		DamageComponent->Activate(true);
	}

	if (UNiagaraComponent* TrailComponent = GetComponentByClass<UNiagaraComponent>())
	{
		TrailComponent->Activate(true);
	}

	ClearComponentOverlaps();

	if (InTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(ActiveTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			Deactivate();
		}), InTime, false);
	}
}

void AWeapon::Deactivate()
{
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj WeaponDeactivate"));

	if (IsValid(DamageComponent))
	{
		DamageComponent->Reset();
	}

	if (UNiagaraComponent* TrailComponent = GetComponentByClass<UNiagaraComponent>())
	{
		TrailComponent->Deactivate();
	}
}

float AWeapon::GetTrailWidth_Implementation()
{
	if (UMeshComponent* MeshComponent = GetComponentByClass<UMeshComponent>())
	{
		if (MeshComponent->DoesSocketExist(TrailStart) && MeshComponent->DoesSocketExist(TrailEnd))
		{
			return FVector::Distance(MeshComponent->GetSocketLocation(TrailStart), MeshComponent->GetSocketLocation(TrailEnd));
		}
	}

	return 0.f;
}

TSubclassOf<UAttributeSet> AWeapon::GetAttributeSet()
{
	return UMWeaponAttributeSet::StaticClass();
}

void AWeapon::SetEquipActor(AActor* EquipActor)
{
	if (IsValid(EquipActor))
	{
		FName SocketName;
		if (AMCharacter* Character = Cast<AMCharacter>(EquipActor))
		{
			SocketName = Character->GetEquipSocketName();
		}
		else
		{
			if (const FWeaponData* WeaponData = GetWeaponData())
			{
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
			}
		}

		if (SocketName != NAME_None)
		{
			AttachToActor(EquipActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
		}
	}

	SetOwner(EquipActor);

	if (AMCharacter* Character = Cast<AMCharacter>(EquipActor))
	{
		EquipChangedHandle = Character->OnWeaponChangedEvent.AddUObject(this, &AWeapon::UpdateAbility);
	}
}

void AWeapon::UpdateAbility(AActor* OldWeapon, AActor* NewWeapon)
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
		CharacterAbilityComponent->GetNumericAttribute(UMAttributeSet::GetAttackSpeedAttribute());
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
}

void AWeapon::ResetCombo()
{
	Combo = INDEX_NONE;
	OnComboChangedEvent.Broadcast(Combo);
}

void AWeapon::FinishCoolDown()
{
	bCoolDown = false;
	OnCoolDownFinishedEvenet.Broadcast();
}

bool AWeapon::IsAttackable() const
{
	const FWeaponData* WeaponData = GetWeaponData();

	if (WeaponData == nullptr)
	{
		return false;
	}

	if (IsCoolDown())
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

int32 AWeapon::GetEffectIndex() const
{
	if (const FWeaponData* WeaponData = GetWeaponData())
	{
		return WeaponData->EffectIndex;
	}

	return INDEX_NONE;
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
