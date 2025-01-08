#include "MBattleComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "Engine/OverlapResult.h"

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
	UE_CLOG(TeamIndex == CheckTeamIndex, LogTemp, Warning, TEXT("같은 팀"));
	UE_CLOG(TeamIndex != CheckTeamIndex, LogTemp, Warning, TEXT("다른 팀"));
	return TeamIndex == CheckTeamIndex;
}
