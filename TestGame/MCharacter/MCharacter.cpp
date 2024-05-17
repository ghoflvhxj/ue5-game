#include "MCharacter.h"

#include "TestGame/TestGame.h"

#include "Component/MBattleComponent.h"
#include "Component/StateMachineComponent.h"

#include "AttributeSet.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"

#include "CharacterState/MCharacterState.h"
#include "TestGame/Mcharacter/MCharacterEnum.h"

#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "OnlineSubsystem.h"
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
}

// Called every frame
void AMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		IInteractInterface* InteractInterface = nullptr;
		if (GetInteractInterface(OtherActor, InteractInterface))
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

	if (IsValid(AbilitySystemComponent))
	{
		bool bHasAuthority = HasAuthority();
		if (bHasAuthority)
		{
			AbilitySystemComponent->SetAvatarActor(this);
		}

		// AbilitySet
		if (IsValid(AbilitySetData))
		{
			AbilitySetData->GiveAbilities(AbilitySystemComponent, AblitiyHandles);
		}

		// AttributeSet
		//if (bHasAuthority)
		{
			AttributeSet = const_cast<UMAttributeSet*>(AbilitySystemComponent->GetSet<UMAttributeSet>());
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AMCharacter::OnHealthChanged);
			UE_LOG(LogTemp, Warning, TEXT("MoveSpeed: %f"), AbilitySystemComponent->GetNumericAttributeBase(AttributeSet->GetMoveSpeedAttribute()));

			GetCharacterMovement()->MaxWalkSpeed = AbilitySystemComponent->GetNumericAttributeBase(AttributeSet->GetMoveSpeedAttribute());
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMoveSpeedAttribute()).AddWeakLambda(this, [this](const FOnAttributeChangeData & AttributeChangeData) {
				GetCharacterMovement()->MaxWalkSpeed = AbilitySystemComponent->GetNumericAttributeBase(AttributeSet->GetMoveSpeedAttribute());
			});
		}
	}
}

	return FGameplayAbilitySpecHandle();
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

void AMCharacter::MoveToLocation()
{
	if (IsValid(AbilitySystemComponent) == false)
	{
		return;
	}

	FGameplayEventData GameplayEventData;
	GameplayEventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Move"));
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = this;


	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Event.Move")), GameplayEventData);
	//FGameplayAbilitySpecHandle AbilityHandle;
	//

	//AbilitySystemComponent->TriggerAbilityFromGameplayEvent()
}

bool AMCharacter::IsInteractableActor(AActor* OtherActor)
{
	if (IsValid(OtherActor) == false)
	{
		return false;
	}

	IInteractInterface* InteractInterface = nullptr;
	if (GetInteractInterface(OtherActor, InteractInterface) == false)
	{
		return false;
	}

	if (InteractInterface->Execute_IsInteractable(OtherActor, this) == false)
	{
		return false;
	}

	InteractInterface->Execute_OnTargeted(OtherActor, this);

	return true;
}

bool AMCharacter::GetInteractInterface(AActor* Actor, IInteractInterface*& OutInterface)
{
	bool bIsInterfaceImplemented = Actor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass());
	if (bIsInterfaceImplemented == false)
	{
		return false;
	}

	OutInterface = Cast<IInteractInterface>(Actor);
	if (OutInterface == nullptr)
	{
		return false;
	}

	return true;
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
