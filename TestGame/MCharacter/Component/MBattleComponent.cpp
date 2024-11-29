#include "MBattleComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "Engine/OverlapResult.h"

#include "TestGame/Bullet/Bullet.h"

UMBattleComponent::UMBattleComponent()
{
	SetComponentTickEnabled(true);

	PrimaryComponentTick.bCanEverTick = false;
}

void UMBattleComponent::Attack(TArray<AActor*> Targets)
{
	if (BulletClass == nullptr || Targets.Num() == 0)
	{
		return;
	}

	TArray<AActor*> PickedTargets;
	if (PickTargets(Targets, PickedTargets))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(BulletClass, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), SpawnParameters);
		FVector Direction = (Targets[0]->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
		//Bullet->Fire(Direction, this, 0);
	}
}

bool UMBattleComponent::IsSameTeam(int CheckTeamIndex) const
{
	UE_CLOG(TeamIndex == CheckTeamIndex, LogTemp, Warning, TEXT("같은 팀"));
	UE_CLOG(TeamIndex != CheckTeamIndex, LogTemp, Warning, TEXT("다른 팀"));
	return TeamIndex == CheckTeamIndex;
}

bool UMBattleComponent::IsSameTeam(UMBattleComponent* OtherBattleComponent) const
{
	if (IsValid(OtherBattleComponent))
	{
		return IsSameTeam(OtherBattleComponent->TeamIndex);
	}

	UE_LOG(LogTemp, Warning, TEXT("실패2"));

	return false;
}

bool UMBattleComponent::IsAttackableTarget(AActor* Target)
{
	if (AMCharacter* MCharacter = Cast<AMCharacter>(Target))
	{
		if (MCharacter == GetOwner())
		{
			return false;
		}

		return true;
	}

	return false;
}

bool UMBattleComponent::PickTargets(TArray<AActor*>& InTargets, TArray<AActor*>& OutPickedTargets)
{
	OutPickedTargets = InTargets;
	return true;
}
