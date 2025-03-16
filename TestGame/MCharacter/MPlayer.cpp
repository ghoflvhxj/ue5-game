#include "MPlayer.h"
#include "MyPlayerState.h"

#include "MGameInstance.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/MAbility/MAbilitySystemComponent.h"
#include "TestGame/MAbility/MAbility.h"
#include "TestGame/MAbility/MActionAbility.h"
#include "TestGame/MHud/MHud.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "SkillSubsystem.h"

//#include "CharacterLevelSubSystem.h"

const FPlayerCharacterTableRow FPlayerCharacterTableRow::Empty = FPlayerCharacterTableRow();

AMPlayer::AMPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	LevelComponent = CreateDefaultSubobject<ULevelComponent>(TEXT("LevelComponent"));

	InputCacheComponent = CreateDefaultSubobject<UInputCacheComponent>(TEXT("InputCacheComponent"));
}

void AMPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &AMPlayer::UpdateGameplayEffect);
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &AMPlayer::RemoveGameplayEffect);
	}

	EquipItem(0);
}

void AMPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMPlayer::SetPlayerDefaults()
{
	Super::SetPlayerDefaults();

	AMPlayerState* MyPlayerState = GetPlayerState<AMPlayerState>();
	if (IsValid(MyPlayerState) == false || IsValid(AbilitySystemComponent) == false || IsValid(AbilitySetData) == false)
	{
		return;
	}

	// 스킬 태그들만
	FGameplayTagContainer SkillTagFilter;
	const FPlayerCharacterTableRow& PCTableRow = UMGameInstance::GetPlayerCharacterTableRow(GetWorld(), MyPlayerState->GetCharacterIndex());
	for (const TPair<int32, FGameplayTag> SkillToAbilityTag : PCTableRow.Skills)
	{
		SkillTagFilter.AddTag(SkillToAbilityTag.Value);
	}

	AbilitySetData->GiveAbilities(AbilitySystemComponent, SkillTagFilter);

	for (const TPair<int32, FGameplayTag> SkillIndexToTag : PCTableRow.Skills)
	{
		if (UGameplayAbility_Skill* SkillAbility = Cast<UGameplayAbility_Skill>(GetAbility(SkillIndexToTag.Value)))
		{
			SkillAbility->SetSkillIndex(SkillIndexToTag.Key, SkillIndexToTag.Value);
		}
	}
}

void AMPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (IsValid(StartInputAction) && IsValid(EnhancedInputComponent))
	{
		InputHandle = &EnhancedInputComponent->BindAction(StartInputAction, ETriggerEvent::Completed, this, &AMPlayer::StopStartAnim);
	}
}

void AMPlayer::BasicAttack()
{
	if(LoadedSkillIndex != INDEX_NONE)
	{
		const FPlayerCharacterTableRow& PCTableRow = UMGameInstance::GetPlayerCharacterTableRow(this, GetPlayerState<AMPlayerState>()->GetCharacterIndex());
		if (PCTableRow.Skills.Contains(LoadedSkillIndex) == false)
		{
			return;
		}

		FGameplayAbilitySpecHandle AbilitySpecHandle = GetAbilitySpecHandle(PCTableRow.Skills[LoadedSkillIndex]);
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			AbilitySystemComponent->AbilityLocalInputPressed(AbilitySpec->InputID);
			return;
		}
	}

	Super::BasicAttack();
}

void AMPlayer::OnStartAnimFinished_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	Super::OnStartAnimFinished_Implementation(Montage, bInterrupted);

	UEnhancedInputComponent* EnhancedInputComponent = GetComponentByClass<UEnhancedInputComponent>();
	if (IsValid(StartInputAction) && IsValid(EnhancedInputComponent) && InputHandle != nullptr)
	{
		EnhancedInputComponent->RemoveBindingByHandle(InputHandle->GetHandle());
		InputHandle = nullptr;
	}
}

void AMPlayer::StopStartAnim()
{
	// 어빌리티 형태로 바꾸어서, ASC에 의해 몽타주 재생 관련 데이터가 레플리케이션 되도록 만들어야 함
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer StartTagContainer(FGameplayTag::RequestGameplayTag("Ability.Start"));
		AbilitySystemComponent->CancelAbilities(&StartTagContainer);
	}
}

UGameplayAbility* AMPlayer::GetLoadedSkillAbility()
{
	return GetSkillAbility(LoadedSkillIndex);
}

UGameplayAbility_Skill* AMPlayer::GetSkillAbility(int32 InSkillIndex)
{
	const FPlayerCharacterTableRow& PCTableRow = UMGameInstance::GetPlayerCharacterTableRow(this, GetPlayerState<AMPlayerState>()->GetCharacterIndex());
	return PCTableRow.Skills.Contains(InSkillIndex) ? Cast<UGameplayAbility_Skill>(GetAbility(PCTableRow.Skills[InSkillIndex])) : nullptr;
}

void AMPlayer::UseSkill(int32 InSkillSlot)
{
	AbilitySystemComponent->AbilityLocalInputPressed(InSkillSlot);
}

void AMPlayer::FinishSkill()
{
	LoadedSkillIndex = INDEX_NONE;
}

void AMPlayer::Server_SkillEnhance_Implementation(int32 InIndex)
{
	UMGameInstance* GameInstance = GetGameInstance<UMGameInstance>();

	if (IsValid(GameInstance) == false || IsValid(AbilitySystemComponent) == false || InIndex == INDEX_NONE)
	{
		return;
	}

	OnSkillEnhancedDelegate.Broadcast(InIndex);
}

void AMPlayer::UpdateGameplayEffect(UAbilitySystemComponent* InAbilitySystemCoponent, const FGameplayEffectSpec& InGameplayEffectSpec, FActiveGameplayEffectHandle InActiveGameplayEffectHandle)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	AMHudInGame* Hud = PlayerController->GetHUD<AMHudInGame>();
	if (IsValid(Hud) == false)
	{
		return;
	}

	if (IsValid(AbilitySystemComponent))
	{
		if (const FActiveGameplayEffect* ActiveGameplayEffect = AbilitySystemComponent->GetActiveGameplayEffect(InActiveGameplayEffectHandle))
		{
			Hud->UpdateByGameplayEffect(AbilitySystemComponent, *ActiveGameplayEffect);
		}
	}
}

void AMPlayer::RemoveGameplayEffect(const FActiveGameplayEffect& InRemovedGameplayEffect)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	AMHudInGame* Hud = PlayerController->GetHUD<AMHudInGame>();
	if (IsValid(Hud) == false)
	{
		return;
	}

	Hud->UpdateByGameplayEffect(GetAbilitySystemComponent(), InRemovedGameplayEffect);
}

int32 AMPlayer::GetEffectIndex(const FGameplayTag& InTag) const
{
	const FPlayerCharacterTableRow& PCTableRow = GetPlayerCharacterTableRow();
	return PCTableRow.MapTagToEffectIndex.Contains(InTag) ? PCTableRow.MapTagToEffectIndex[InTag] : INDEX_NONE;
}

FPlayerCharacterTableRow AMPlayer::GetPlayerCharacterTableRow() const
{
	if (AMPlayerState* MPlayerState = GetPlayerState<AMPlayerState>())
	{
		return UMGameInstance::GetPlayerCharacterTableRow(GetWorld(), MPlayerState->GetCharacterIndex());
	}
	
	return FPlayerCharacterTableRow::Empty;
}

int32 AMPlayer::GetCharacterIndex()
{
	if (AMPlayerState* MPlayerState = GetPlayerState<AMPlayerState>())
	{
		return MPlayerState->GetCharacterIndex();
	}

	return INDEX_NONE;
}

UInputCacheComponent::UInputCacheComponent()
{
	SetIsReplicatedByDefault(false);
	PrimaryComponentTick.bCanEverTick = false;
}

void UInputCacheComponent::SetInput(const FVector& InVector)
{
	LastInputVector = InVector;
}

void UInputCacheComponent::UpdateStack(const FVector& InVector)
{
	Stack = (LastInputVector == InVector) ? Stack + 1 : 1;
	LastInputVector = InVector;
}

int UInputCacheComponent::GetStackCount()
{
	UE_LOG(LogTemp, Warning, TEXT("BlueprintGetter Test"));
	return Stack;
}
