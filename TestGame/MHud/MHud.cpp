#include "MHud.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMHud::BeginPlay()
{
	Super::BeginPlay();

	CreateHUDWidget();
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

	if (AMCharacter* PlayerCharacter = Cast<AMCharacter>(PlayerOwner->GetCharacter()))
	{
		UpdateCharacterInfo(nullptr, PlayerCharacter);

		PlayerCharacter->OnWeaponChangedEvent.AddUObject(this, &AMHudInGame::UpdateWeaponInfo);
	}

	PlayerOwner->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::UpdateCharacterInfo);
	
	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			RoundComponent->OnRoundChangedEvent.AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}
	}
}
