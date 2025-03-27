#include "MCharacter.h"

#include "AttributeSet.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
// 임시
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "TestGame/TestGame.h"
#include "TestGame/MAbility/MAbilitySystemComponent.h"
#include "TestGame/MAbility/MAbility.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "Component/MBattleComponent.h"
#include "Component/StateMachineComponent.h"
#include "Component/ActionComponent.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MGameMode/MGameModeInGame.h"
#include "TestGame/MCharacter/Component/InteractorComponent.h"
#include "TestGame/MCharacter/MCharacterEnum.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MPlayerController/MPlayerController.h"
#include "CharacterState/MCharacterState.h"

#include "MGameInstance.h"

// Sets default values
AMCharacter::AMCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	TeamComponent = CreateDefaultSubobject<UMTeamComponent>(TEXT("TeamComponent"));

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		NiagaraComponent->SetupAttachment(MeshComponent);
	}

	ActionComponent = CreateDefaultSubobject<UMActionComponent>(TEXT("ActionComponent"));
}

UMInventoryComponent* AMCharacter::GetInventoryComponent() const
{
	if (IsValid(InventoryComponent))
	{
		return InventoryComponent;
	}

	if (APlayerState* TempPlayerState = GetPlayerState())
	{
		return TempPlayerState->GetComponentByClass<UMInventoryComponent>();
	}

	return nullptr;
}

// Called when the game starts or when spawned
void AMCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AbilitySystemComponent) && IsValid(AbilitySetData))
	{
		if (HasAuthority())
		{
			AbilitySystemComponent->SetAvatarActor(this);
			AddAbilities(AbilitySetData);
		}
		 
		AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("Character.Freeze"), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AMCharacter::Freeze);

		if (AbilitySetData->HasAbilityTag(FGameplayTag::RequestGameplayTag("Ability.Start")) == false)
		{
			OnStartAnimFinished(nullptr, false);
		}

		AttributeSet = const_cast<UMAttributeSet*>(AbilitySystemComponent->GetSet<UMAttributeSet>());
		if (IsValid(AttributeSet))
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMoveSpeedAttribute()).AddUObject(this, &AMCharacter::OnMoveSpeedChanged);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AMCharacter::OnHealthChanged);

			TArray<FGameplayAttribute> Attributes;
			AbilitySystemComponent->GetAllAttributes(Attributes);

			for (FGameplayAttribute& Attribute : Attributes)
			{
				float AttributeValue = Attribute.GetNumericValue(AttributeSet);

				FOnAttributeChangeData AttributeChangeData;
				AttributeChangeData.Attribute = Attribute;
				AttributeChangeData.OldValue = AttributeValue;
				AttributeChangeData.NewValue = AttributeValue;

				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).Broadcast(AttributeChangeData);
			}
		}
	}

	if (USkeletalMeshComponent* SkeletalmeshComponent = GetMesh())
	{
		SkeletalmeshComponent->SetAllBodiesSimulatePhysics(false);

		if (UAnimInstance* AnimInst = SkeletalmeshComponent->GetAnimInstance())
		{
		}
	}
}

// Called every frame
void AMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractTargets.Num() > 0 && IsPlayerControlled())
	{
		TWeakObjectPtr<AActor> ClosetTarget = InteractTargets[0];
		for (auto Iterator = InteractTargets.CreateIterator(); Iterator; ++Iterator)
		{
			if (Iterator->IsValid() == false)
			{
				Iterator.RemoveCurrent();
				continue;
			}

			if ((*Iterator)->GetDistanceTo(this) < ClosetTarget->GetDistanceTo(this))
			{
				ClosetTarget = *Iterator;
			}
		}

		if (ClosetTarget.IsValid())
		{
			if (UMInteractorComponent* InteractorComponent = ClosetTarget->FindComponentByClass<UMInteractorComponent>())
			{
				InteractorComponent->Interact(this);
				InteractTargets.Remove(ClosetTarget);
			}
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (IsPlayerControlled() && IsLocallyControlled())
		{
			SetMoving(MovementComponent->GetLastInputVector() != FVector::ZeroVector);
		}
	}
}

// Called to bind functionality to input
void AMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 총 쏠때 사용한거
		//EnhancedInputComponent->BindAction(InputAction3, ETriggerEvent::Triggered, this, &AMCharacter::Aim);

		// 마우스 왼쪽 때면 공격
		EnhancedInputComponent->BindAction(InputAction2, ETriggerEvent::Completed, this, &AMCharacter::BasicAttack);
		// 마우스 왼쪽 1초 누르면 차징
		//EnhancedInputComponent->BindAction(InputAction3, ETriggerEvent::Started, this, &AMCharacter::ChargeInputPressed);
		//EnhancedInputComponent->BindAction(InputAction3, ETriggerEvent::Triggered, this, &AMCharacter::ChargeLightAttack);
		// 마우스 왼쪽 1초 누른거 때면 차징 공격
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMCharacter::LightChargeAttack);
	}
}

void AMCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsValid(OtherActor) == false)
	{
		return;
	}

	if (IsPlayerControlled())
	{
		if (UMInteractorComponent* InteractComponent = OtherActor->GetComponentByClass<UMInteractorComponent>())
		{
			if (InteractComponent->IsInteractable())
			{
				//if (InteractComponent->IsChannel(EInteractChannel::Unique))
				//{
				//	InteractTargets.AddUnique(OtherActor);
				//}

				InteractComponent->Interact(this);
			}
		}
	}
}

void AMCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (InteractTargets.Contains(OtherActor))
	{
		if (IInteractInterface* InteractInterface = GetInteractInterface(OtherActor))
		{
			InteractInterface->Execute_OnUnTargeted(OtherActor, this);
		}

		InteractTargets.Remove(OtherActor);
	}
}

float AMCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	//if (IsState<ECharacterVitalityState>(ECharacterVitalityState::Die))
	//{
	//	return 0.f;
	//}

	//UE_LOG(LogTemp, Warning, TEXT("Take Damaged!!! HP:%f"), NewHealth);

	return Damage;
}

void AMCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMCharacter, TargetAngle, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AMCharacter, bAimMode, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AMCharacter, bMoving, COND_SimulatedOnly);
	//DOREPLIFETIME_CONDITION(AMCharacter, bRotateToTargetAngle, COND_SimulatedOnly);
	DOREPLIFETIME(AMCharacter, Item);
}

UAbilitySystemComponent* AMCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMCharacter::OnAbilityInputPressed(int32 InInputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InInputID);
	}
}

void AMCharacter::OnAbilityInputReleased(int32 InInputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(InInputID);
	}
}

void AMCharacter::RemoveAbilities(UMAbilityDataAsset* AbilityDataAsset)
{
	if (IsValid(AbilityDataAsset))
	{
		AbilityDataAsset->ClearAbilities(AbilitySystemComponent);
	}
}

FGameplayAbilitySpecHandle AMCharacter::GetAbilitySpecHandle(FGameplayTag InTag)
{
	static FGameplayAbilitySpecHandle EmptyAbilitySpecHandle;

	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	AbilitySystemComponent->FindAllAbilitiesWithTags(AbilitySpecHandles, InTag.GetSingleTagContainer());

	if (AbilitySpecHandles.Num() > 0)
	{
		return AbilitySpecHandles[0];
	}

	return EmptyAbilitySpecHandle;
}

UGameplayAbility* AMCharacter::GetAbility(FGameplayTag InTag)
{
	if (IsValid(AbilitySystemComponent))
	{
		const FGameplayAbilitySpecHandle& AbilitySpecHandle = GetAbilitySpecHandle(InTag);

		if (AbilitySpecHandle.IsValid())
		{
			if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
			{
				return AbilitySpec->GetPrimaryInstance();
			}
		}
	}

	return nullptr;
}

void AMCharacter::MakeEffectCue(const FGameplayTag& InTag)
{
	MakeEffectCue(GetEffectIndex(InTag));
}

void AMCharacter::MakeEffectCue(int32 InEffectIndex)
{
	if (InEffectIndex == INDEX_NONE)
	{
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.EffectContext = AbilitySystemComponent->MakeEffectContext(InEffectIndex);
	CueParams.Instigator = this;
	CueParams.EffectCauser = this;
	CueParams.Location = GetActorLocation();

	AbilitySystemComponent->ExecuteGameplayCueLocal(FGameplayTag::RequestGameplayTag("GameplayCue.Effect"), CueParams);
}

void AMCharacter::ChargeInputPressed()
{
	//if (IsValid(AbilitySystemComponent))
	//{
	//	bChargeInput = AbilitySystemComponent->GetTagCount(FGameplayTag::RequestGameplayTag("Action.Attack.Block")) == 0;
	//}
}

void AMCharacter::AddAbilities(UMAbilityDataAsset* InAbilitySet)
{
	if (IsValid(InAbilitySet) == false)
	{
		return;
	}

	InAbilitySet->GiveAbilities(AbilitySystemComponent);
}

void AMCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	GetCharacterMovement()->MaxWalkSpeed = AttributeChangeData.NewValue;
}

void AMCharacter::OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (IsValid(AbilitySystemComponent) == false)
	{
		return;
	}

	APawn* DamageInstigator = nullptr;
	if (AttributeChangeData.GEModData)
	{
		DamageInstigator = Cast<APawn>(AttributeChangeData.GEModData->EffectSpec.GetEffectContext().GetInstigator());
	}

	if (AttributeChangeData.NewValue < AttributeChangeData.OldValue && AttributeChangeData.OldValue > 0.f && AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetMaxHealthAttribute()) != AttributeChangeData.NewValue)
	{
		OnDamaged(DamageInstigator);
	}

	if (AttributeChangeData.NewValue == 0.f && AttributeChangeData.OldValue > 0.f)
	{
		OnDeadEvent.Broadcast();
		if (AMGameModeInGame* GameMode = Cast<AMGameModeInGame>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->OnPawnKilled(DamageInstigator, this);
		}

		MakeEffectCue(FGameplayTag::RequestGameplayTag("Character.Dead"));

		if (AController* MyController = GetController())
		{
			if (UBehaviorTreeComponent* BrainComponent = MyController->GetComponentByClass<UBehaviorTreeComponent>())
			{
				BrainComponent->StopLogic(TEXT("Dead"));
			}
			MyController->StopMovement();
		}

		if (IsPlayerControlled() == false && HasAuthority())
		{
			SetLifeSpan(3.f);
		}

		if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
		{
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
		}

		FTimerHandle RagdollStartTimerHandle;
		GetWorldTimerManager().SetTimer(RagdollStartTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
			Ragdoll();
		}), 1.f, false);

		TArray<UPrimitiveComponent*> Primitives;
		GetComponents<UPrimitiveComponent>(Primitives);
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			Primitive->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			Primitive->SetGenerateOverlapEvents(false);
		}

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Character.Dead"));
		}
	}

	//if (IsNetMode(NM_DedicatedServer) == false)
	//{
		//UpdateHealthbarWidget(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
	//}
}

void AMCharacter::OnDamaged(AActor* DamageInstigator)
{
	FGameplayTag DamagedTag = FGameplayTag::RequestGameplayTag("Character.Damaged");

	if (IsValid(AbilitySystemComponent))
	{
		FGameplayEventData GameplayEventData;
		GameplayEventData.EventTag = DamagedTag;
		GameplayEventData.Instigator = DamageInstigator;
		GameplayEventData.Target = this;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, DamagedTag, GameplayEventData);
	}

	if (IsNetMode(NM_DedicatedServer) == false)
	{
		Opacity = 0.3f;
		ActionNumber = 0;

		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]() {

			Opacity = (FMath::Cos((2.f * PI) * (ActionNumber / 10.f)) + 3.f) * 2.f / 10.f;

			SetMaterialParam([this](UMaterialInstanceDynamic* DynamicMaterialInstance) {
				DynamicMaterialInstance->SetScalarParameterValue("Opacity", Opacity);
				DynamicMaterialInstance->SetVectorParameterValue("Emissive", FVector4((30 - ActionNumber) / 30.f, 0.f, 0.f, 0.f));
			});

			++ActionNumber;

			if (ActionNumber == 30)
			{
				GetWorldTimerManager().ClearTimer(Handle);
			}
		}), 0.1f, true);

		//if (IsDead() == false)
		//{
			MakeEffectCue(DamagedTag);
		//}
	}
}

float AMCharacter::GetAttribute(const FName& InAttributeName)
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayAttribute FoundAttribute;

		if (InAttributeName == UMAttributeSet::GetAttackSpeedAttribute().AttributeName)
		{
			FoundAttribute = UMAttributeSet::GetAttackSpeedAttribute();
		}

		if (FoundAttribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(FoundAttribute);
		}
	}

	return 0.f;
}

bool AMCharacter::IsDead()
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->GetSpawnedAttributes().Num() > 0)
		{
			return AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetHealthAttribute()) == 0.f;
		}

		return false;
	}
	
	return true;
}

bool AMCharacter::IsSameTeam(AActor* OtherCharacter) const
{
	if (AMCharacter* OtherMCharacter = Cast<AMCharacter>(OtherCharacter))
	{
		if (IsValid(TeamComponent) && IsValid(OtherMCharacter->TeamComponent))
		{
			return TeamComponent->IsSameTeam(OtherMCharacter->TeamComponent);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("실패1"));

	return false;
}

UPrimitiveComponent* AMCharacter::GetWeaponCollision()
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		return Weapon->GetComponentByClass<UCapsuleComponent>();
	}

	return nullptr;
}

void AMCharacter::SetWeaponActivation(bool bInActivation, float InTime)
{
	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	if (bInActivation)
	{
		if (InTime > 0.f)
		{
			InTime /= GetAttribute(TEXT("AttackSpeed"));
		}

		Weapon->Activate(InTime);
	}
	else
	{
		Weapon->Deactivate();
	}
}

void AMCharacter::SetWeaponCollisionEnable_Implementation(bool bInEnable)
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		Weapon->SetActorEnableCollision(bInEnable);
	}
}

bool AMCharacter::GetWeaponMuzzleTransform(FTransform& OutTransform)
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		return Weapon->GetMuzzleTransform(OutTransform);
	}

	return false;
}

bool AMCharacter::IsWeaponEquipped() const
{
	return true;
}

void AMCharacter::EquipItem(int32 InItemIndex)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (AWeapon* CurrentWeapon = Cast<AWeapon>(Item))
	{
		if (CurrentWeapon->GetWeaponIndex() == InItemIndex)
		{
			return;
		}
	}

	const FWeaponData& WeaponTableRow = UMGameInstance::GetWeaponTableRow(this, InItemIndex);
	WeaponTableRow.WeaponClass.LoadSynchronous();
	if (AWeapon* NewWeapon = Cast<AWeapon>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, WeaponTableRow.WeaponClass.Get(), GetActorTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this)))
	{
		NewWeapon->SetInstigator(this);
		NewWeapon->SetWeaponIndex(InItemIndex);
		UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
		NewWeapon->SetEquipActor(this);

		OnWeaponChangedEvent.Broadcast(Item, NewWeapon);

		Item = NewWeapon;
	}
}

FName AMCharacter::GetEquipSocketName()
{
	return EquipTypeToSocektName.Contains(TEXT("Weapon")) ? EquipTypeToSocektName[TEXT("Weapon")] : NAME_None;
}

bool AMCharacter::GetWeaponData(FWeaponData& OutWeaponData)
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		OutWeaponData = *Weapon->GetWeaponData();
		return true;
	}

	return false;
}

void AMCharacter::OnRep_Weapon(AActor* OldWeapon)
{
	//if (IsLocallyControlled() && IsValid(WeaponCached))
	//{
	//	WeaponCached->SetActorHiddenInGame(true);
	//	WeaponCached->SetLifeSpan(0.1f);
	//	WeaponCached = nullptr;
	//}

	if (IsValid(Item) && IsValid(ActionComponent))
	{
		ActionComponent->UpdateAction(Item->GetComponentByClass<UMActionComponent>());
	}

	OnWeaponChangedEvent.Broadcast(OldWeapon, Item);
}

void AMCharacter::AddItem(int32 InIndex, int32 InNum)
{
	UMInventoryComponent* InventoryComp = GetInventoryComponent();
	if (IsValid(InventoryComp) == false)
	{
		return;
	}

	InventoryComp->AddItem(InIndex, InNum);
}

void AMCharacter::UseItem(int32 InIndex)
{
	OnItemUsedEvent.Broadcast(InIndex);
}

void AMCharacter::Aim()
{
	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	const FWeaponData* WeaponData = Weapon->GetWeaponData();
	if (WeaponData == nullptr)
	{
		return;
	}

	if (Weapon->IsAttackable() && WeaponData->WeaponType == EWeaponType::Gun)
	{
		switch (WeaponData->WeaponRotateType)
		{
		case EWeaponRotateType::None:
		{
			// 회전하지 않음
		}
		break;
		case EWeaponRotateType::Instantly:
		{
			LookMouse(-1.f);
		}
		break;
		case EWeaponRotateType::Smoothly:
		{
			LookMouse(5.f);
		}
		break;
		}
	}
}

void AMCharacter::BasicAttack()
{
	if (IsValid(AbilitySystemComponent) == false)
	{
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Character.Attack.Block")))
	{
		return;
	}

	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	if (Weapon->IsAttackable())
	{
		if (ComboReserveHandle.IsValid())
		{
			Weapon->GetCoolDownFinishedEvent().Remove(ComboReserveHandle);
			ComboReserveHandle.Reset();
		}

		AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(EAbilityInputType::Attack));
		AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(EAbilityInputType::Combo));
	}
	else if(ComboReserveHandle.IsValid() == false && Weapon->IsComboable())
	{
		ComboReserveHandle = Weapon->GetCoolDownFinishedEvent().AddUObject(this, &AMCharacter::BasicAttack);
	}
}

void AMCharacter::FinishBasicAttack()
{
	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(1);
		AbilitySystemComponent->AbilityLocalInputReleased(2);
	}

	SetRotateToTargetAngle(false);
}

void AMCharacter::ChargeLightAttack()
{
	//if (bChargeInput == false)
	//{
	//	return;
	//}

	//AWeapon* Weapon = GetEquipItem<AWeapon>();
	//if (IsValid(Weapon) == false || Weapon->IsCharged())
	//{
	//	return;
	//}

	//if (IsValid(AbilitySystemComponent) == false || AbilitySystemComponent->GetTagCount(FGameplayTag::RequestGameplayTag("Action.Attack.Block")))
	//{
	//	return;
	//}

	//AbilitySystemComponent->AbilityLocalInputPressed(5);

	//UE_LOG(LogTemp, Warning, TEXT("Charge"));
}

void AMCharacter::LightChargeAttack()
{
	FinishCharge();

	// 이 함수는 손을 때는 순간 호출되므로, 차징 공격 중에 클릭하여 bChargeInput이 false로 변해도 이 함수는 한참전에 호출됬을거임
	if (AbilitySystemComponent && bChargeInput)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(5);
		AbilitySystemComponent->AbilityLocalInputPressed(2);
	}
}

void AMCharacter::FinishCharge()
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		Weapon->UnCharge();
	}

	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->Deactivate();
	}

	UE_LOG(LogTemp, Warning, TEXT("FinishCharge"));
}

void AMCharacter::LookMouse(float TurnSpeed)
{
	AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(GetController());
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	float Angle = PlayerController->GetYawToMouse();
	float CurrentAngle = GetActorRotation().Yaw;
	float DeltaAngle = FMath::UnwindDegrees(Angle - CurrentAngle);

	float ResultAngle = TurnSpeed < 0.f ? Angle : FMath::FInterpTo(CurrentAngle, CurrentAngle + DeltaAngle, GetWorld()->GetDeltaSeconds(), TurnSpeed);

	if (IsLocallyControlled())
	{
		Server_SetTargetAngle(ResultAngle, true);

		FVector Line = { 300.f, 0.f, 0.f };
		Line = UKismetMathLibrary::RotateAngleAxis(Line, Angle, FVector::UpVector);
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Line, FColor::Cyan, false, 3.f, 0, 3.f);
	}

	SetActorRotation(FRotator(0.f, ResultAngle, 0.f));
}

void AMCharacter::Server_SetTargetAngle_Implementation(float InTargetAngle, bool bInstantly)
{
	TargetAngle = InTargetAngle;
	if (bInstantly)
	{
		SetActorRotation(FRotator(0.f, InTargetAngle, 0.f));
	}
}

void AMCharacter::SetRotateToTargetAngle(bool bNewValue)
{
	//bRotateToTargetAngle = bNewValue;
	//Server_SetRotateToTargetAngle(bRotateToTargetAngle);

	//if (GetLocalRole() == ROLE_AutonomousProxy)
	//{
	//	UpdateTargetAngle();
	//}
}

void AMCharacter::Server_SetRotateToTargetAngle_Implementation(bool bNewValue)
{
	//bRotateToTargetAngle = bNewValue;
}

void AMCharacter::OnRep_TargetAngle()
{
	//UE_LOG(LogTemp, Warning, TEXT("TargetAngle OnRep"));
	//RotateToTargetAngle();
}

void AMCharacter::RotateToTargetAngle()
{
	//if (IsValid(Weapon) && Weapon->GetEquipItemData() != nullptr)
	//{
	//	switch (Weapon->GetEquipItemData()->WeaponRotateType)
	//	{
	//		default:
	//		case EWeaponRotateType::Instantly:
	//		{
	//			FRotator Rotator;
	//			Rotator.Yaw = TargetAngle;
	//			SetActorRotation(Rotator);
	//		}
	//		break;
	//		case EWeaponRotateType::Smoothly:
	//		{	
	//			FRotator CurrentRot = GetActorRotation();
	//			if (CurrentRot.Yaw != TargetAngle)
	//			{
	//				FVector XDirectVector = { 1.0, 0.0, 0.0 };
	//				FRotator Rotator;
	//				Rotator.Yaw = TargetAngle >= 0.f ? TargetAngle : TargetAngle + 360.f;
	//				float RotScale = GetActorForwardVector().Cross(Rotator.Vector()).Z >= 0.f ? -1.f : 1.f;

	//				FRotator AddRot = FRotator::ZeroRotator;
	//				AddRot.Yaw = 360.f * GetWorld()->GetDeltaSeconds() * RotScale;
	//				
	//				AddActorWorldRotation(AddRot);

	//				if ((RotScale > 0.f && GetActorRotation().Yaw > TargetAngle) || (RotScale < 0.f && GetActorRotation().Yaw < TargetAngle))
	//				{
	//					Rotator = FRotator::ZeroRotator;
	//					Rotator.Yaw = TargetAngle;
	//					SetActorRotation(Rotator);
	//				}
	//			}
	//		}
	//		break;
	//	}
	//}
}

void AMCharacter::KnockBack(const FVector& InLocation, float Radius, float Strength)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->AddRadialImpulse(InLocation, Radius, Strength, ERadialImpulseFalloff::RIF_Linear, false);
	}
}

void AMCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue /*= 1.0f*/, bool bForce /*= false*/)
{
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Character.Move.Block")) && bForce == false)
	{
		return;
	}

	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
}

void AMCharacter::MoveToLocation()
{
	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Controller.MouseRightClick")), GameplayEventData);
}

void AMCharacter::SetMoving(bool bInMoving)
{
	if (bMoving == bInMoving)
	{
		return;
	}

	bMoving = bInMoving;
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_SetMoving(bInMoving);
	}
}

void AMCharacter::Server_SetMoving_Implementation(bool bInMoving)
{
	//UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Server SerMove : %d"), (int)bInMoving);
	bMoving = bInMoving;
}

IInteractInterface* AMCharacter::GetInteractInterface(AActor* Actor)
{
	if (Actor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		return Cast<IInteractInterface>(Actor);
	}

	return nullptr;
}

void AMCharacter::Ragdoll()
{
	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	if (IsValid(SkeletalMeshComponent) && SkeletalMeshComponent->IsSimulatingPhysics() == false)
	{
		if (SkeletalMeshComponent->GetSingleNodeInstance())
		{
			SkeletalMeshComponent->Stop();
		}
		else if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.f);
		}
		SkeletalMeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		SkeletalMeshComponent->SetAllBodiesSimulatePhysics(true);
		SkeletalMeshComponent->SetSimulatePhysics(true);
		SkeletalMeshComponent->WakeAllRigidBodies();
		SkeletalMeshComponent->bBlendPhysics = true;
	}
}

void AMCharacter::PlayStartAnim()
{
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Start")));
}

void AMCharacter::OnStartAnimFinished_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	//bLevelStartFinished = true;
	OnStartFinishedDelegate.Broadcast();
}

void AMCharacter::Server_HasBegunPlay_Implementation()
{
	//if (IsValid(AbilitySystemComponent))
	//{
	//	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Start")));
	//}
}

void AMCharacter::AddEffectStack_Implementation(FGameplayTag InTag)
{

}

void AMCharacter::Server_AimMode_Implementation(bool InAimMode)
{
	bAimMode = InAimMode;
	OnRep_AimMode();
}

void AMCharacter::AimMode()
{
	bAimMode = true;
	OnRep_AimMode();

	if (GetLocalRole() < ROLE_Authority)
	{
		Server_AimMode(true);
	}
}

void AMCharacter::MoveMode()
{
	bAimMode = false;
	OnRep_AimMode();

	if (GetLocalRole() < ROLE_Authority)
	{
		Server_AimMode(false);
	}
}

void AMCharacter::OnRep_AimMode()
{
	if (bAimMode)
	{
		//bUseControllerRotationYaw = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		//bUseControllerRotationYaw = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}