#include "MHud.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
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

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	ensure(IsValid(PlayerController));

	ACharacter* PlayerCharacter = PlayerController->GetCharacter();
	if (IsValid(PlayerCharacter))
	{
		BeginPlayWithCharacter(nullptr, PlayerCharacter);
	}
	else
	{
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::BeginPlayWithCharacter);
	}

	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);
	}
}
