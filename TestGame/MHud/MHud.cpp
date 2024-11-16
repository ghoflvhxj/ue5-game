#include "MHud.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

void AMHud::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(PlayerOwner))
	{
		OnPawnChanged(nullptr, PlayerOwner->GetPawn());

		PlayerOwner->OnPossessedPawnChanged.AddDynamic(this, &AMHud::OnPawnChanged);
	}

	CreateHUDWidget();
}

bool AMHud::InitializeUsingPlayerState(APlayerState* PlayerState)
{
	return IsValid(PlayerState);
}

void AMHud::CreateHUDWidget()
{
	if (IsValid(HUDWidget))
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	if (IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget(GetWorld(), HUDWidgetClass);
	}

	if (IsValid(HUDWidget) && ShowHudWidgetAfterCreation)
	{
		ShowWidget();
	}
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

	if (IsValid(PlayerOwner))
	{
		if (AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerOwner->GetCharacter()))
		{
			UpdateCharacterInfo(nullptr, PlayerCharacter);

			PlayerCharacter->OnWeaponChangedEvent.AddUObject(this, &AMHudInGame::UpdateWeaponInfo);
		}

		PlayerOwner->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::UpdateCharacterInfo);
	}

	
	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			RoundComponent->OnRoundChangedEvent.AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}
	}

	if (APawn* Pawn = GetOwningPawn())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).AddUObject(this, &AMHudInGame::UpdateHealth);
		}
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

	UpdateCharacterInfo(OldPawn, NewPawn);
	
	if (IsValid(NewPawn))
	{
		//if (ULevelComponent* LevelComponent = NewPawn->GetComponentByClass<ULevelComponent>())
		//{
		//	UpdateCharacterExperience(LevelComponent->GetCurrentExperience());
		//	LevelComponent->OnExperienceChangedEvent.AddUObject(this, &AMHudInGame::UpdateCharacterExperience);
		//}
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
