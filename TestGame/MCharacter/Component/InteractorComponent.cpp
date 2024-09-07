#include "InteractorComponent.h"

UMInteractorComponent::UMInteractorComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMInteractorComponent, InteratingActor, COND_SimulatedOnly);
}

void UMInteractorComponent::Indicate(AActor* InActor)
{

}

bool UMInteractorComponent::Interact(AActor* InActor)
{
	if (IsValid(InteratingActor) && InteratingActor != InActor)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		ChangeInteractor(InActor);
		if (InteractStartEvent.IsBound())
		{
			InteractStartEvent.Broadcast();
		}

		if (FMath::IsNearlyZero(InteractData.InteractingTime))
		{
			if (InteractFinishEvent.IsBound())
			{
				InteractFinishEvent.Broadcast();
			}
		}
		else
		{
			World->GetTimerManager().SetTimer(InteractTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
				if (InteractFinishEvent.IsBound())
				{
					InteractFinishEvent.Broadcast();
				}
				ChangeInteractor(nullptr);
			}), FMath::Max(0.1f, InteractData.InteractingTime), false);
		}
		return true;
	}

	return false;
}

void UMInteractorComponent::CancelInteract()
{
	ChangeInteractor(nullptr);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractTimerHandle);
	}
}

void UMInteractorComponent::ChangeInteractor(AActor* InActor)
{
	InteratingActor = InActor;

	if (GetOwnerRole() == ENetRole::ROLE_AutonomousProxy)
	{
		Server_ChangeInteractor(InActor);
	}

	//if (IsInteractableActor(OtherActor))
	//{
	//	TWeakObjectPtr<AActor> ClosetTarget = OtherActor;
	//	if (InteractTargets.Num() > 0)
	//	{
	//		for (auto Iterator = InteractTargets.CreateIterator(); Iterator; ++Iterator)
	//		{
	//			if (Iterator->IsValid() == false)
	//			{
	//				Iterator.RemoveCurrent();
	//				continue;
	//			}

	//			if ((*Iterator)->GetDistanceTo(this) < ClosetTarget->GetDistanceTo(this))
	//			{
	//				ClosetTarget = *Iterator;
	//			}
	//		}
	//	}

	//	InteractTargets.AddUnique(OtherActor);

	//	if (ClosetTarget.IsValid())
	//	{
	//		if (IInteractInterface* InteractInterface = GetInteractInterface(OtherActor))
	//		{
	//			InteractInterface->Execute_OnTargeted(OtherActor, this);
	//		}
	//	}
	//}
}

void UMInteractorComponent::Server_ChangeInteractor_Implementation(AActor* InActor)
{
	ChangeInteractor(InActor);
}

bool UMInteractorComponent::IsInteractingActor(AActor* InActor)
{
	return InActor == InteratingActor;
}
