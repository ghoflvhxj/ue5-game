#include "MyPlayerState.h"

void AMPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerState, bDead);
}

void AMPlayerState::Die()
{
	bDead = true;
	OnRep_Dead();
}

void AMPlayerState::OnRep_Dead()
{
	if (bDead)
	{
		OnPlayerDeadEvent.Broadcast();
	}
}


