#include "InteractorComponent.h"

UMInteractorComponent::UMInteractorComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMInteractorComponent, InteratingActor);
}

void UMInteractorComponent::Indicate(AActor* InActor)
{

}

bool UMInteractorComponent::Interact(AActor* InActor)
{
	if (IsValid(InActor) == false)
	{
		return false;
	}

	if (IsValid(InteratingActor))
	{
		if (InteratingActor != InActor)
		{
			return false;
		}

		if (InteratingActor == InActor && InteractState == EInteractState::Doing)
		{
			return false;
		}
	}

	AddInteractor(InActor);
	if (InteractStartEvent.IsBound())
	{
		InteractStartEvent.Broadcast();
	}

	InteractState = EInteractState::Doing;

	if (bFinishInstantly)
	{
		SuccessInteract();
	}

	return true;
}

void UMInteractorComponent::SuccessInteract()
{
	FinishInteract(true);
}

void UMInteractorComponent::CancelInteract()
{
	AddInteractor(nullptr);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractTimerHandle);
	}

	FinishInteract(false);
}

void UMInteractorComponent::FinishInteract(bool bSuccess)
{
	InteractState = EInteractState::Wait;

	if (InteractFinishEvent.IsBound())
	{
		InteractFinishEvent.Broadcast(bSuccess);
	}
}

void UMInteractorComponent::AddInteractor(AActor* InActor)
{
	if (IsNetSimulating() == false)
	{
		InteratingActor = InActor;
	}
}

void UMInteractorComponent::Server_ChangeInteractor_Implementation(AActor* InActor)
{
	// 클라가 요청한 거면 거리 검사라도 추가 해야함. 일단 아래 코드는 주석처리
	//AddInteractor(InActor);
}

bool UMInteractorComponent::IsInteractingActor(AActor* InActor)
{
	return InActor == InteratingActor;
}

void UMInteractorComponent::OnRep_Interactor()
{
	if (InteractState == EInteractState::Wait)
	{
		Interact(InteratingActor);
	}
}
