#include "MyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TestGame/MHud/MHud.h"

void AMPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HudTimerHandle, FTimerDelegate::CreateUObject(this, &AMPlayerState::AddToHUD), 1.f, true);
	}
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

	if (CharacterIndex == INDEX_NONE)
	{
		return;
	}

	if (IsValid(GetPawn()) == false)
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

	APlayerState* MyPlayerState = MyPlayerController->GetPlayerState<APlayerState>();
	if (IsValid(MyPlayerState) && MyPlayerState != this)
	{
		Hud->AddOtherPlayer(this);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HudTimerHandle);
		HudTimerHandle.Invalidate();
	}
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


