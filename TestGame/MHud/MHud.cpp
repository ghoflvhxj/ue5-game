#include "MHud.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MSpawner/MonsterSpawner.h"
//#include "CharacterLevelSubSystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

void AMHud::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget(GetWorld(), HUDWidgetClass);
	}

	if (IsValid(HUDWidget) && ShowHudWidgetAfterCreation)
	{
		ShowWidget();
	}

	if (IsValid(PlayerOwner))
	{
		OnPawnChanged(nullptr, PlayerOwner->GetPawn());
		PlayerOwner->OnPossessedPawnChanged.AddDynamic(this, &AMHud::OnPawnChanged);
	}
}

bool AMHud::InitializeUsingPlayerState(APlayerState* PlayerState)
{
	return IsValid(PlayerState);
}

void AMHud::ShowWidget()
{
	FTimerHandle DummyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DummyTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (IsValid(HUDWidget))
		{
			HUDWidget->AddToViewport();
		}
	}), ShowTimer + 1.f, false, -1.f);
}

void AMHudInGame::ShowGameOver_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!!!"));
}

AMHudInGame::AMHudInGame()
{
	ShowHudWidgetAfterCreation = false;
}

void AMHudInGame::BeginPlay()
{
	Super::BeginPlay();

	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			UpdateRoundInfo(RoundComponent->GetRoundWave());
			RoundComponent->GetWaitNextRoundEvent().AddUObject(this, &AMHudInGame::ShowGetRewardMessage);
			RoundComponent->OnRoundAndWaveChangedEvent.AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}

		GameStateInGame->OnBossMonsterSet.AddUObject(this, &AMHudInGame::BoundBoss);
		GameStateInGame->OnMatchEndEvent.AddUObject(this, &AMHudInGame::ShowGameFinish);
	}
}

bool AMHudInGame::InitializeUsingPlayerState(APlayerState* PlayerState)
{
	if (Super::InitializeUsingPlayerState(PlayerState))
	{
		if (UMInventoryComponent* InventoryComponent = PlayerState->GetComponentByClass<UMInventoryComponent>())
		{
			InventoryComponent->OnMoneyChangedEvent.AddUObject(this, &AMHudInGame::UpdateMoney);
		}

		return true;
	}

	return false;
}

void AMHudInGame::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	Super::OnPawnChanged(OldPawn, NewPawn);

	if (AMCharacter* PlayerCharacter = Cast<AMCharacter>(NewPawn))
	{
		UpdateCharacterInfo(nullptr, PlayerCharacter);

		PlayerCharacter->OnWeaponChangedEvent.AddUObject(this, &AMHudInGame::UpdateWeaponInfo);
	}

	if (IsValid(OldPawn))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = OldPawn->GetComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).Remove(HealthUpdateHandle);
		}
	}

	if (IsValid(NewPawn))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = NewPawn->GetComponentByClass<UAbilitySystemComponent>())
		{
			HealthUpdateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).AddUObject(this, &AMHudInGame::UpdateHealth);
			UpdateHealthProxy(0.f, AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetHealthAttribute()), AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetMaxHealthAttribute()));
		}
	}
}

void AMHudInGame::UpdateHealth(const FOnAttributeChangeData& AttributeChangeData)
{
	if (APawn* Pawn = GetOwningPawn())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetComponentByClass<UAbilitySystemComponent>())
		{
			UpdateHealthProxy(AttributeChangeData.OldValue, AttributeChangeData.NewValue, AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetMaxHealthAttribute()));
		}
	}
}
