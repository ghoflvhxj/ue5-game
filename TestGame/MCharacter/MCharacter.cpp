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

// 임시
#include "BehaviorTree/BehaviorTreeComponent.h"

// Sets default values
AMCharacter::AMCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SearchComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SearchComponent"));
	SearchComponent->SetupAttachment(GetRootComponent());

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

	if (bRotateToTargetAngle)
	{
		RotateToTargetAngle();
	}

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

	//PlayerInputComponent->BindAction(FName(TEXT("MouseLeftClick")), EInputEvent::IE_Released, this, &AMCharacter::MoveToMouseLocation);
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
	DOREPLIFETIME_CONDITION(AMCharacter, bRotateToTargetAngle, COND_SimulatedOnly);
	DOREPLIFETIME(AMCharacter, Weapon);
}

UAbilitySystemComponent* AMCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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

	if (AttributeChangeData.NewValue < AttributeChangeData.OldValue)
	{
		OnDamaged();
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
					GameMode->OnPawnKilled(Cast<APawn>(AttributeChangeData.GEModData->EffectSpec.GetEffectContext().GetInstigator()), this);
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
			AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Character.State.Dead"));
		}
	}

	if (IsNetMode(NM_DedicatedServer) == false)
	{
		UpdateHealthbarWidget(AttributeChangeData.OldValue, AttributeChangeData.NewValue);
	}
}

void AMCharacter::OnDamaged()
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayEventData GameplayEventData;
		GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Event.Damaged"));
		GameplayEventData.Instigator = this;
		GameplayEventData.Target = this;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag("Character.Event.Damaged"), GameplayEventData);
	}
}

UPrimitiveComponent* AMCharacter::GetComponentForAttackSearch()
{
	return SearchComponent;
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
	if (IsValid(Weapon))
	{
		return Weapon->GetMuzzleTransform(OutTransform);
	}

	return false;
}

bool AMCharacter::IsWeaponEquipped() const
{
	return true;
}

void AMCharacter::EquipWeapon(AWeapon* InWeapon)
{
	if (Weapon == InWeapon)
	{
		return;
	}

	AWeapon* OldWeapon = Weapon;

	if (IsValid(Weapon))
	{
		Weapon->SetActorHiddenInGame(true);
	}

	if (HasAuthority())
	{
		Weapon = InWeapon;
		OnRep_Weapon(OldWeapon);
	}
	
	if (HasAuthority() == false && InWeapon->HasAuthority())
	{
		if (IsValid(WeaponCached))
		{
			WeaponCached->SetActorHiddenInGame(true);
			WeaponCached->SetLifeSpan(0.1f);
		}
		
		WeaponCached = InWeapon;
	}

	if (IsValid(InWeapon))
	{
		InWeapon->OnEquipped(this);
	}

	if (OnWeaponChangedEvent.IsBound())
	{
		OnWeaponChangedEvent.Broadcast(OldWeapon, InWeapon);
	}
}

void AMCharacter::OnRep_Weapon(AWeapon* OldWeapon)
{
	if (IsLocallyControlled() && IsValid(WeaponCached))
	{
		WeaponCached->SetActorHiddenInGame(true);
		WeaponCached->SetLifeSpan(0.1f);
		WeaponCached = nullptr;
	}

	if (IsValid(Weapon))
	{
		if (UMActionComponent* ActionComponent = GetComponentByClass<UMActionComponent>())
		{
			ActionComponent->UpdateAction(Weapon->ActionComponent);
		}
	}

	if (OnWeaponChangedEvent.IsBound())
	{
		OnWeaponChangedEvent.Broadcast(OldWeapon, Weapon);
	}
}

void AMCharacter::TryBasicAttack()
{
	if (IsAttackable() == false)
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> ActiveAbilities;
	AbilitySystemComponent->FindAllAbilitiesWithTags(ActiveAbilities, FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Character.Action.BasicAttack")));
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : ActiveAbilities)
	{
		AbilitySystemComponent->CancelAbilityHandle(AbilitySpecHandle);
	}

	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag("Controller.MouseLeftClick"), GameplayEventData);
}

void AMCharacter::StartBasicAttack()
{
	if (IsAttackable() == false)
	{
		return;
	}

	UpdateTargetAngle();
	SetRotateToTargetAngle(true);

	Weapon->BasicAttack();
}

void AMCharacter::FinishBasicAttack()
{

}

bool AMCharacter::IsAttackable()
{
	return IsValid(Weapon) && Weapon->IsAttackable();
}

void AMCharacter::UpdateTargetAngle()
{
	if (IsLocallyControlled())
	{
		float NewTargetAngle = 0.f;
		// 공격 시의 마우스 위치 시각화
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
					DrawDebugSphere(GetWorld(), HitResults[0].Location, 100.f, 32, FColor::Red);

					NewTargetAngle = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResults[0].Location).Yaw;
				}
			}
		}

		TargetAngle = NewTargetAngle;
		Server_UpdateTargetAngle(TargetAngle);
	}
}

void AMCharacter::Server_UpdateTargetAngle_Implementation(float InTargetAngle)
{
	TargetAngle = InTargetAngle;
}

void AMCharacter::SetRotateToTargetAngle(bool bNewValue)
{
	bRotateToTargetAngle = bNewValue;
	Server_SetRotateToTargetAngle(bRotateToTargetAngle);

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UpdateTargetAngle();
	}
}

void AMCharacter::Server_SetRotateToTargetAngle_Implementation(bool bNewValue)
{
	bRotateToTargetAngle = bNewValue;
}

void AMCharacter::OnRep_TargetAngle()
{
	UE_LOG(LogTemp, Warning, TEXT("TargetAngle OnRep"));
	RotateToTargetAngle();
}

void AMCharacter::RotateToTargetAngle()
{
	if (IsValid(Weapon))
	{
		switch (Weapon->GetWeaponData()->WeaponRotateType)
		{
			default:
			case EWeaponRotateType::Instantly:
			{
				FRotator Rotator;
				Rotator.Yaw = TargetAngle;
				SetActorRotation(Rotator);
			}
			break;
			case EWeaponRotateType::Smoothly:
			{	
				FRotator CurrentRot = GetActorRotation();
				if (CurrentRot.Yaw != TargetAngle)
				{
					FVector XDirectVector = { 1.0, 0.0, 0.0 };
					FRotator Rotator;
					Rotator.Yaw = TargetAngle >= 0.f ? TargetAngle : TargetAngle + 360.f;
					float RotScale = GetActorForwardVector().Cross(Rotator.Vector()).Z >= 0.f ? -1.f : 1.f;

					FRotator AddRot = FRotator::ZeroRotator;
					AddRot.Yaw = 360.f * GetWorld()->GetDeltaSeconds() * RotScale;
					
					AddActorWorldRotation(AddRot);

					if ((RotScale > 0.f && GetActorRotation().Yaw > TargetAngle) || (RotScale < 0.f && GetActorRotation().Yaw < TargetAngle))
					{
						Rotator = FRotator::ZeroRotator;
						Rotator.Yaw = TargetAngle;
						SetActorRotation(Rotator);
					}
				}
			}
			break;
		}
	}
}

void AMCharacter::MoveToLocation()
{
	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Send Move Event"));
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
