#pragma once

#include "MCue.h"
#include "GameplayTags.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MHud/MHud.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MAttribute/MAttribute.h"

DECLARE_LOG_CATEGORY_CLASS(LogCue, Log, Log);

void UGameplayCue_FloatMessage::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(MyTarget) && IsValid(PlayerController) && PlayerController->IsLocalController())
	{
		if (AMHudInGame* HudInGame = PlayerController->GetHUD<AMHudInGame>())
		{
			HudInGame->ShowFloatingMessage(MyTarget, Parameters);
		}
	}
}

AGameplayCue_CounterAttack::AGameplayCue_CounterAttack()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(SphereComponent);
}

bool AGameplayCue_CounterAttack::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (CueInstigator.IsValid())
	{
		FVector Offset = FVector::ZeroVector;
		if (ACharacter* Character = Cast<ACharacter>(CueInstigator))
		{
			if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
			{
				Offset.Z += CapsuleComponent->GetScaledCapsuleHalfHeight();
			}
		}

		SetActorLocation(CueInstigator->GetActorLocation() + Offset);
	}

	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->ResetSystem();
	}

	return true;
}
