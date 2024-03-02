#include "MHud.h"
#include "TestGame/MGameState/MGameStateInGame.h"

void AMHud::BeginPlay()
{
	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHud::ShowGameOver);
	}
}

void AMHud::ShowGameOver_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!!!"));
}

