#include "MBattleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "TestGame/MCharacter/MCharacter.h"

#include "TestGame/Bullet/Bullet.h"

UMTeamComponent::UMTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMTeamComponent, TeamIndex);
}

void UMTeamComponent::SetTeamIndex(int32 InTeamIndex)
{
	TeamIndex = InTeamIndex;
}

bool UMTeamComponent::IsSameTeam(AActor* OtherActor) const
{
	if (IsValid(OtherActor))
	{
		return IsSameTeam(OtherActor->GetComponentByClass<UMTeamComponent>());
	}

	return false;
}

bool UMTeamComponent::IsSameTeam(UMTeamComponent* OtherBattleComponent) const
{
	if (IsValid(OtherBattleComponent))
	{
		return IsSameTeam(OtherBattleComponent->TeamIndex);
	}

	return false;
}

bool UMTeamComponent::IsSameTeam(int CheckTeamIndex) const
{
	return TeamIndex == CheckTeamIndex;
}
