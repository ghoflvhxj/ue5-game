#include "MCharacter.h"

#include "TestGame/TestGame.h"

#include "Component/MBattleComponent.h"
#include "Component/StateMachineComponent.h"
#include "Component/ActionComponent.h"
#include "TestGame/MGameMode/MGameModeInGame.h"
#include "TestGame/MCharacter/Component/InteractorComponent.h"

#include "AttributeSet.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"

#include "CharacterState/MCharacterState.h"
#include "TestGame/Mcharacter/MCharacterEnum.h"

#include "TestGame/MWeapon/Weapon.h"

#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"

// 임시
#include "BehaviorTree/BehaviorTreeComponent.h"

// Sets default values
AMCharacter::AMCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	BattleComponent = CreateDefaultSubobject<UMBattleComponent>(TEXT("BattleComponent"));

	StateComponent = CreateDefaultSubobject<UStateComponent>(TEXT("StateComponent"));
}

// Called when the game starts or when spawned
void AMCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AbilitySystemComponent))
	{
		if (HasAuthority())
		{
			AbilitySystemComponent->SetAvatarActor(this);

			if (IsValid(AbilitySetData))
			{
				AbilitySetData->GiveAbilities(AbilitySystemComponent, AblitiyHandles);
			}
		}
		else
		{

		}

		AttributeSet = const_cast<UMAttributeSet*>(AbilitySystemComponent->GetSet<UMAttributeSet>());
		if (ensure(AttributeSet))
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
}

// Called every frame
void AMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (bRotateToTargetAngle)
	//{
	//	RotateToTargetAngle();
	//}

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
}

// Called to bind functionality to input
void AMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//if (PlayerGameplayAbilitiesDataAsset)
		{
			//const TSet<FGameplayInputAbilityInfo>& InputAbilities = PlayerGameplayAbilitiesDataAsset->GetInputAbilities();
			//const TSet<FGameplayInputAbilityInfo>& InputAbilities;
			//for (const auto& It : InputAbilities)
			//{
				//if (It.IsValid())
				//{
					//const UInputAction* InputAction = It.InputAction;
					//const int32 InputID = It.InputID;

					if (InputAction)
					{
						EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMCharacter::OnAbilityInputPressed, 0);
						EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMCharacter::OnAbilityInputReleased, 0);
					}


					EnhancedInputComponent->BindAction(InputAction2, ETriggerEvent::Triggered, this, &AMCharacter::BasicAttack);
					EnhancedInputComponent->BindAction(InputAction2, ETriggerEvent::Completed, this, &AMCharacter::FinishBasicAttack);
				//}
			//}
		}
	}
}

void AMCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsInteractableActor(OtherActor))
	{
		InteractTargets.AddUnique(OtherActor);
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
	//DOREPLIFETIME_CONDITION(AMCharacter, bRotateToTargetAngle, COND_SimulatedOnly);
	DOREPLIFETIME(AMCharacter, Item);
}

UAbilitySystemComponent* AMCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMCharacter::OnAbilityInputPressed(int32 InInputID)
{
	if (InInputID == 1)
	{
		BasicAttack();
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InInputID);
	}
}

void AMCharacter::OnAbilityInputReleased(int32 InInputID)
{
	if (InInputID == 1)
	{
		FinishBasicAttack();
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(InInputID);
	}
}

void AMCharacter::AddAbilities(UMAbilityDataAsset* AbilityDataAsset)
{
	if (IsValid(AbilityDataAsset))
	{
		AbilityDataAsset->GiveAbilities(AbilitySystemComponent, AblitiyHandles);
	}
}

void AMCharacter::RemoveAbilities(UMAbilityDataAsset* AbilityDataAsset)
{
	if (IsValid(AbilityDataAsset))
	{
		AbilityDataAsset->ClearAbilities(AbilitySystemComponent, AblitiyHandles);
	}
}

void AMCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
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

	if (AttributeChangeData.NewValue < AttributeChangeData.OldValue)
	{
		OnDamaged(DamageInstigator);
	}

	if (AttributeChangeData.NewValue <= 0.f)
	{
		if (HasAuthority())
		{
			if (IsValid(StateComponent))
			{
				StateComponent->ChangeState<ECharacterVitalityState>(ECharacterVitalityState::Die);
			}

			if (AMGameModeInGame* GameMode = Cast<AMGameModeInGame>(UGameplayStatics::GetGameMode(this)))
			{
				if (AttributeChangeData.GEModData)
				{
					GameMode->OnPawnKilled(DamageInstigator, this);
				}
			}

			if (AController* MyController = GetController())
			{
				if (UBehaviorTreeComponent* BrainComponent = MyController->GetComponentByClass<UBehaviorTreeComponent>())
				{
					BrainComponent->StopLogic(TEXT("Dead"));
				}
				
				MyController->StopMovement();
			}

			if (UAnimMontage* Montage = GetCurrentMontage())
			{
				StopAnimMontage(Montage);
			}

			SetLifeSpan(3.f);
		}

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

	if (IsNetMode(NM_DedicatedServer) == false)
	{
		UpdateHealthbarWidget(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
	}
}

void AMCharacter::OnDamaged(AActor* DamageInstigator)
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayEventData GameplayEventData;
		GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Event.Damaged"));
		GameplayEventData.Instigator = DamageInstigator;
		GameplayEventData.Target = this;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag("Character.Event.Damaged"), GameplayEventData);
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
	}
}

void AMCharacter::TestFunction1()
{

}

void AMCharacter::TestFunction2()
{

}

void AMCharacter::TestFunction3()
{

}

void AMCharacter::AddVitalityChangedDelegate(UObject* Object, const TFunction<void(uint8, uint8)> Function)
{
	CompoentLogic<UStateComponent>([Object, Function](UStateComponent* StateMachineComponent){
		StateMachineComponent->AddOnStateChangeDelegate(UCharacterVitalityState::StaticClass(), Object, Function);
	});
}

bool AMCharacter::IsDead()
{
	bool bResult = false;

	CompoentLogic<UStateComponent>([&bResult](UStateComponent* StateMachineComponent) {
		bResult = StateMachineComponent->GetState<ECharacterVitalityState>() == ECharacterVitalityState::Die;
	});

	return bResult;
}

bool AMCharacter::IsSameTeam(AActor* OtherCharacter) const
{
	if (AMCharacter* OtherMCharacter = Cast<AMCharacter>(OtherCharacter))
	{
		if (IsValid(BattleComponent) && IsValid(OtherMCharacter->BattleComponent))
		{
			return BattleComponent->IsSameTeam(OtherMCharacter->BattleComponent);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("실패1"));

	return false;
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

void AMCharacter::EquipItem(AActor* InItem)
{
	if (Item != InItem)
	{
		AActor* OldWeapon = Item;
		Item = InItem;
		OnWeaponChangedEvent.Broadcast(OldWeapon, InItem);
	}
}

bool AMCharacter::GetWeaponData(FWeaponData& OutWeaponData)
{
	if (AWeapon* Weapon = GetEquipItem<AWeapon>())
	{
		OutWeaponData = *Weapon->GetItemData();
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

	if (IsValid(Item))
	{
		if (UMActionComponent* ActionComponent = GetComponentByClass<UMActionComponent>())
		{
			ActionComponent->UpdateAction(Item->GetComponentByClass<UMActionComponent>());
		}
	}

	OnWeaponChangedEvent.Broadcast(OldWeapon, Item);
}

void AMCharacter::BasicAttack()
{
	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	// 공격 방향으로 회전
	// 공격 시의 마우스 위치 시각화
	float NewTargetAngle = GetActorRotation().Yaw;
	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		FVector MouseWorldLocation;
		FVector MouseWorldDirection;
		FCollisionObjectQueryParams CollsionParam;
		CollsionParam.AddObjectTypesToQuery(ECC_WorldStatic);
		CollsionParam.AddObjectTypesToQuery(ECC_WorldDynamic);

		if (PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
		{
			TArray<FHitResult> HitResults;
			if (GetWorld()->LineTraceMultiByObjectType(HitResults, MouseWorldLocation, MouseWorldLocation + MouseWorldDirection * 10000.f, CollsionParam))
			{
				//DrawDebugSphere(GetWorld(), HitResults[0].Location, 100.f, 32, FColor::Red);
				NewTargetAngle = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResults[0].Location).Yaw;
			}
		}
	}

	if (Weapon->IsAttackable())
	{
		if (const FWeaponData* WeaponData = Weapon->GetItemData())
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
					if (IsNetMode(NM_DedicatedServer) == false)
					{
						Server_SetTargetAngle(NewTargetAngle, true);
					}
					SetActorRotation(FRotator(0.f, NewTargetAngle, 0.f));
				}
				break;
				case EWeaponRotateType::Smoothly:
				{
					float CurrentYaw = GetActorRotation().Yaw;
					float DeltaYaw = FMath::UnwindDegrees(NewTargetAngle - CurrentYaw);
					float InterpSpeed = 5.f;

					SetActorRotation(FRotator(0.f, FMath::FInterpTo(CurrentYaw, CurrentYaw + DeltaYaw, GetWorld()->GetDeltaSeconds(), InterpSpeed), 0.f));
					Server_SetTargetAngle(GetActorRotation().Yaw, true);
				}
				break;
			}

			if (WeaponData->MoveSpeed == 0.f && IsValid(Controller))
			{
				Controller->StopMovement();
			}
		}
	}

	if (Weapon->IsCoolDown() == false)
	{
		if (Weapon->IsAttackable())
		{
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->AbilityLocalInputPressed(1);
				AbilitySystemComponent->AbilityLocalInputReleased(2);
			}
		}
		else
		{
			FinishBasicAttack();
		}
	}
}

void AMCharacter::FinishBasicAttack()
{
	AWeapon* Weapon = GetEquipItem<AWeapon>();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(1);
		AbilitySystemComponent->AbilityLocalInputPressed(2);
	}

	SetRotateToTargetAngle(false);
}

void AMCharacter::TurnToWeaponAim()
{

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

void AMCharacter::MoveToLocation()
{
	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Controller.MouseRightClick")), GameplayEventData);
}

bool AMCharacter::IsInteractableActor(AActor* OtherActor)
{
	if (IsValid(OtherActor))
	{
		//if (IInteractInterface * InteractInterface = GetInteractInterface(OtherActor))
		//{
		//	return InteractInterface->Execute_IsInteractable(OtherActor, this);
		//}

		return IsValid(OtherActor->FindComponentByClass<UMInteractorComponent>());
	}

	return false;
}

IInteractInterface* AMCharacter::GetInteractInterface(AActor* Actor)
{
	if (Actor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		return Cast<IInteractInterface>(Actor);
	}

	return nullptr;
}

void AMCharacter::LeaderboardTest()
{
//	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
//	IOnlineLeaderboardsPtr Leaderboards = OnlineSubsystem->GetLeaderboardsInterface();
//	if (Leaderboards == nullptr)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("리더보드 인터페이스 없음"));
//		return;
//	}
//\
//	ReadObject = MakeShareable(new FOnlineLeaderboardRead());
//	FOnlineLeaderboardReadRef ReadObjectRef = ReadObject.ToSharedRef();
//	ReadObjectRef->LeaderboardName = FName(TEXT("GM0002Single"));
//
//	LeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &AMCharacter::PrintLeaderboard);
//	FDelegateHandle LeaderboardReadCompleteDelegateHandle = Leaderboards->AddOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegate);
//	Leaderboards->ReadLeaderboardsAroundRank(5, 3, ReadObjectRef);
}

void AMCharacter::PrintLeaderboard(bool b)
{
	//UE_LOG(LogTemp, Warning, TEXT("리더보드 프린트"));

	//for (int32 RowIdx = 0; RowIdx < ReadObject->Rows.Num(); ++RowIdx)
	//{
	//	const FOnlineStatsRow& StatsRow = ReadObject->Rows[RowIdx];
	//	UE_LOG_ONLINE_LEADERBOARD(Log, TEXT("   Leaderboard stats for: Nickname = %s, Rank = %d"), *StatsRow.NickName, StatsRow.Rank);

	//	for (FStatsColumnArray::TConstIterator It(StatsRow.Columns); It; ++It)
	//	{
	//		UE_LOG_ONLINE_LEADERBOARD(Log, TEXT("     %s = %s"), *It.Key().ToString(), *It.Value().ToString());
	//	}
	//}
}
