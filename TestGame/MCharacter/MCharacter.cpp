#include "MCharacter.h"

#include "TestGame/TestGame.h"

#include "Component/MBattleComponent.h"
#include "Component/StateMachineComponent.h"

#include "AttributeSet.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"

#include "CharacterState/MCharacterState.h"
#include "TestGame/Mcharacter/MCharacterEnum.h"

#include "TestGame/MWeapon/Weapon.h"

#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
//#include "OnlineSubsystemUtils.h"


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
		TWeakObjectPtr<AActor> ClosetTarget = OtherActor;
		if (InteractTargets.Num() > 0)
		{
			for (auto Iterator = InteractTargets.CreateIterator(); Iterator; ++Iterator)
			{
				if (Iterator->IsValid() == false)
				{
					Iterator.RemoveCurrent();
				}

				if ((*Iterator)->GetDistanceTo(this) < ClosetTarget->GetDistanceTo(this))
				{
					ClosetTarget = *Iterator;
				}
			}
		}

		InteractTargets.AddUnique(OtherActor);

		if (ClosetTarget.IsValid())
		{
			if (IInteractInterface* InteractInterface = GetInteractInterface(OtherActor))
			{
				InteractInterface->Execute_OnTargeted(OtherActor, this);
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
	DOREPLIFETIME_CONDITION(AMCharacter, bRotateToTargetAngle, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(AMCharacter, Weapon, COND_InitialOnly);
}

}


void AMCharacter::EffectTest()
{
	UGameplayEffect* NewEffect = NewObject<UGameplayEffect>();
	NewEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
	//NewEffect->DurationMagnitude = FGameplayEffectModifierMagnitude(1.f);

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = FGameplayAttribute(UMAttributeSet::StaticClass()->FindPropertyByName("Health"));
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(100.f);
	NewEffect->Modifiers.Add(ModifierInfo);

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);

	//FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UGameplayEffect, 1, EffectContextHandle);

	AbilitySystemComponent->ApplyGameplayEffectToSelf(NewEffect, 0, EffectContextHandle);
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

	if (FMath::IsNearlyEqual(FMath::Max(AttributeChangeData.NewValue, 0.f), 0.f))
	{
		if (UStateComponent* StateMachineComponent = GetComponentByClass<UStateComponent>())
		{
			StateMachineComponent->ChangeState<ECharacterVitalityState>(ECharacterVitalityState::Die);
			SetActorEnableCollision(false);
			//SetLifeSpan(0.1f);
		}
	}

	float MaxHealth = AbilitySystemComponent->GetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute());
	//CreateFloaterWidget(AttributeChangeData.OldValue, AttributeChangeData.NewValue); // 체력이 
	//UpdateHealthbarWidget(); // HUD를 가져와서 작업되도록 변경하기

	OnHealthChangedDelegate.Broadcast(AttributeChangeData.OldValue, AttributeChangeData.NewValue, MaxHealth);
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

bool AMCharacter::IsWeaponEquipped() const
{
	return true;
}

void AMCharacter::EquipWeapon(AWeapon* InWeapon)
{
	if (HasAuthority() == false || Weapon == InWeapon)
	{
		return;
	}

	Multicast_EquipWeapon(InWeapon);
}

void AMCharacter::Multicast_EquipWeapon_Implementation(AWeapon* InWeapon)
{
	Weapon = InWeapon;

	if (UMActionComponent* ActionComponent = GetComponentByClass<UMActionComponent>())
	{
		ActionComponent->UpdateAction(Weapon->ActionComponent);
	}
}

bool AMCharacter::IsAttackable()
{
	//ItemEquipComponent
	if (Weapon.IsValid())
	{
		return Weapon->IsAttackable();
	}

	return false;
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
	if (Weapon.IsValid())
	{
		//Weapon->
	}

	FRotator Rotator;
	Rotator.Yaw = TargetAngle;
	SetActorRotation(Rotator);
}

void AMCharacter::MoveToLocation()
{
	if (IsValid(AbilitySystemComponent) == false)
	{
		return;
	}

	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Send Move Event"));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Move")), GameplayEventData);
}

bool AMCharacter::IsInteractableActor(AActor* OtherActor)
{
	if (IsValid(OtherActor))
	{
		if (IInteractInterface * InteractInterface = GetInteractInterface(OtherActor))
		{
			return InteractInterface->Execute_IsInteractable(OtherActor, this);
		}
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
