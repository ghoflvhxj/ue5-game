#include "MyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TestGame/MHud/MHud.h"
#include "TestGame/MCharacter/MCharacter.h"

void AMPlayerState::BeginPlay()
{
	Super::BeginPlay();
	AddToHUD();
}

void AMPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerState, bDead);
	DOREPLIFETIME(AMPlayerState, CharacterIndex);
}

void AMPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AMPlayerState* MPlayerState = Cast<AMPlayerState>(PlayerState))
	{
		MPlayerState->bDead = bDead;
		MPlayerState->CharacterIndex = CharacterIndex;
	}
}

void AMPlayerState::AddToHUD()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (HudTimerHandle.IsValid())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(HudTimerHandle, FTimerDelegate::CreateUObject(this, &AMPlayerState::AddToHUDInternal), 1.f, true);
}

void AMPlayerState::AddToHUDInternal()
{
	if (CharacterIndex == INDEX_NONE)
	{
		return;
	}

	AMCharacter* Character = GetPawn<AMCharacter>();
	if (IsValid(Character) == false)
	{
		return;
	}

	APlayerController* MyPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(MyPlayerController) == false)
	{
		return;
	}

	AMHudInGame* Hud = MyPlayerController->GetHUD<AMHudInGame>();
	if (IsValid(Hud) == false)
	{
		return;
	}

	Hud->AddPlayer(this, Character);
	GetWorldTimerManager().ClearTimer(HudTimerHandle);
	HudTimerHandle.Invalidate();
}

void AMPlayerState::Die()
{
	bDead = true;
	OnRep_Dead();

	if (UWorld* World = GetWorld())
	{
		if (AMGameStateInGame* GameState = World->GetGameState<AMGameStateInGame>())
		{
			GameState->AddDeadPlayer(this);
		}
	}
}

void AMPlayerState::OnRep_Dead()
{ 
	if (bDead)
	{
		OnPlayerDeadEvent.Broadcast();
	}
}


