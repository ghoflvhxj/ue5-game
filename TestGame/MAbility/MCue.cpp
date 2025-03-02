#pragma once

#include "MCue.h"
#include "GameplayTags.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include "TestGame/MHud/MHud.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MGameState/MGameStateInGame.h"

DECLARE_LOG_CATEGORY_CLASS(LogCue, Log, Log);

bool UGameplayCue_FloatMessage::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(MyTarget) && IsValid(LocalPlayerController))
	{
		if (AMHudInGame* HudInGame = LocalPlayerController->GetHUD<AMHudInGame>())
		{
			HudInGame->ShowFloatingMessage(MyTarget, Parameters);
		}
	}

	return true;
}

bool UGameplayCue_StatusEffect::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget)) // Super::OnActive_Implementation가 무조건 false임. 왜...? 거기서 MyTarget검사를 했으면 좋을탠데
	{
		APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (bOnlyAutonomous)
		{
			if (IsValid(LocalPlayerController) && LocalPlayerController->GetViewTarget() != MyTarget)
			{
				return false;
			}
		}

		FGameplayCueUIData EffectStatusData;
		EffectStatusData.Target = MyTarget;
		EffectStatusData.GameplayCueParams = Parameters;

		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyTarget))
		{
			TArray<FActiveGameplayEffectHandle> ActiveEffectHandles = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(Parameters.AggregatedSourceTags));
			if (ActiveEffectHandles.Num() > 0)
			{
				EffectStatusData.ActiveGameplayEffectHandle = ActiveEffectHandles[0];
				if (const FActiveGameplayEffect* ActiveGameplayEffect = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandles[0]))
				{
					EffectStatusData.Value = ActiveGameplayEffect->Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Effect.Value"));
				}
			}
		}

		if (AMHudInGame* HudInGame = LocalPlayerController->GetHUD<AMHudInGame>())
		{
			//Widget = HudInGame->AddStatusEffect(MyTarget, Parameters.MatchedTagName);
			//if (Widget->GetClass()->ImplementsInterface(UObjectByGameplayCue::StaticClass()))
			//{
			//	IObjectByGameplayCue::Execute_InitByGameplayCue(Widget.Get(), EffectStatusData);
			//}
		}

		return true;
	}

	return false;
}

bool UGameplayCue_StatusEffect::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget)) // Super::OnActive_Implementation가 무조건 false임. 왜...? 거기서 MyTarget검사를 했으면 좋을탠데
	{
		APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (bOnlyAutonomous)
		{
			if (IsValid(LocalPlayerController) && LocalPlayerController->GetViewTarget() != MyTarget)
			{
				return false;
			}
		}

		FGameplayCueUIData EffectStatusData;
		EffectStatusData.Target = MyTarget;
		EffectStatusData.GameplayCueParams = Parameters;

		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyTarget))
		{
			float StartTime = 0.f;
			TArray<FActiveGameplayEffectHandle> ActiveEffectHandles = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(Parameters.MatchedTagName.GetSingleTagContainer()));
			for(const FActiveGameplayEffectHandle& ActiveEffectHandle : ActiveEffectHandles)
			{
				if (const FActiveGameplayEffect* ActiveGameplayEffect = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle))
				{
					if (ActiveGameplayEffect->StartServerWorldTime > StartTime)
					{
						EffectStatusData.ActiveGameplayEffectHandle = ActiveEffectHandle;
						StartTime = ActiveGameplayEffect->StartServerWorldTime;
					}

					for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : ActiveGameplayEffect->Spec.ModifiedAttributes)
					{
						EffectStatusData.Value += UAbilitySystemBlueprintLibrary::GetModifiedAttributeMagnitude(ActiveGameplayEffect->Spec, ModifiedAttribute.Attribute);
					}
				}
			}
		}

		if (AMHudInGame* HudInGame = LocalPlayerController->GetHUD<AMHudInGame>())
		{
			//Widget = HudInGame->AddStatusEffect(MyTarget, Parameters.MatchedTagName);
			//if (Widget->GetClass()->ImplementsInterface(UObjectByGameplayCue::StaticClass()))
			//{
			//	IObjectByGameplayCue::Execute_InitByGameplayCue(Widget.Get(), EffectStatusData);
			//}
		}

		return true;
	}

	return false;
}

bool UGameplayCue_StatusEffect::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget)) // Super::OnActive_Implementation가 무조건 false임. 왜...? 거기서 MyTarget검사를 했으면 좋을탠데
	{
		APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (bOnlyAutonomous)
		{
			if (IsValid(LocalPlayerController) && LocalPlayerController->GetViewTarget() != MyTarget)
			{
				return false;
			}
		}

		FGameplayCueUIData EffectStatusData;
		EffectStatusData.Target = MyTarget;
		EffectStatusData.GameplayCueParams = Parameters;

		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyTarget))
		{
			float StartTime = 0.f;
			TArray<FActiveGameplayEffectHandle> ActiveEffectHandles = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(Parameters.MatchedTagName.GetSingleTagContainer()));
			for (const FActiveGameplayEffectHandle& ActiveEffectHandle : ActiveEffectHandles)
			{
				if (const FActiveGameplayEffect* ActiveGameplayEffect = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle))
				{
					if (ActiveGameplayEffect->StartServerWorldTime > StartTime)
					{
						EffectStatusData.ActiveGameplayEffectHandle = ActiveEffectHandle;
						StartTime = ActiveGameplayEffect->StartServerWorldTime;
					}

					for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : ActiveGameplayEffect->Spec.ModifiedAttributes)
					{
						EffectStatusData.Value += UAbilitySystemBlueprintLibrary::GetModifiedAttributeMagnitude(ActiveGameplayEffect->Spec, ModifiedAttribute.Attribute);
					}
				}
			}
		}

		if (AMHudInGame* HudInGame = LocalPlayerController->GetHUD<AMHudInGame>())
		{
			//Widget = HudInGame->AddStatusEffect(MyTarget, Parameters.MatchedTagName);
			//if (Widget->GetClass()->ImplementsInterface(UObjectByGameplayCue::StaticClass()))
			//{
			//	IObjectByGameplayCue::Execute_InitByGameplayCue(Widget.Get(), EffectStatusData);
			//}
		}

		return true;
	}

	return true;
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
	SetActorLocation(Parameters.Location);

	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->ResetSystem();
	}

	return true;
}

bool UGameplayCue_CascadeParticle::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget) == false)
	{
		return false;
	}

	USceneComponent* AttachComponent = MyTarget->GetRootComponent();
	if (ACharacter* Character = Cast<AMCharacter>(MyTarget))
	{
		AttachComponent = Character->GetMesh();
	}

	if (IsValid(Particle))
	{
		UFXSystemComponent* FXSystemComponent = Test.FindOrAdd(MyTarget);

		if (IsValid(FXSystemComponent) == false)
		{
			if (Particle->IsA<UParticleSystem>())
			{
				FXSystemComponent = UGameplayStatics::SpawnEmitterAttached(Cast<UParticleSystem>(Particle), AttachComponent, SocketName);
			}
			else if (Particle->IsA<UNiagaraSystem>())
			{
				FXSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(Cast<UNiagaraSystem>(Particle), AttachComponent, SocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::Type::SnapToTarget, true, true, ENCPoolMethod::AutoRelease);
			}
			Test.Emplace(MyTarget, FXSystemComponent);
		}
	}

	if (IsValid(SoundHelperClass))
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.Owner = MyTarget;
		if (AActor* SoundHelper = MyTarget->GetWorld()->SpawnActor<AActor>(SoundHelperClass, ActorSpawnParams))
		{
			MapSoundHelper.Emplace(MyTarget, SoundHelper);
		}
	}

	return true;
}

bool UGameplayCue_CascadeParticle::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (Test.Contains(MyTarget))
	{
		//if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(Test[MyTarget]))
		//{
		//	NiagaraComponent->DeactivateImmediate();
		//}
		if (UFXSystemComponent* FXSystemComponent = Test[MyTarget])
		{
			FXSystemComponent->Deactivate();
		}

		Test.Remove(MyTarget);
	}

	if (MapSoundHelper.Contains(MyTarget))
	{
		if (AActor* SoundHelper = MapSoundHelper[MyTarget].Get())
		{
			SoundHelper->Destroy();
		}
		MapSoundHelper.Remove(MyTarget);
	}

	return true;
}

bool UGameplayCue_SkillCoolDown::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget) == false)
	{
		return false;
	}

	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn* ViewTarget = Cast<APawn>(LocalPlayerController->GetViewTarget());
	if (IsValid(LocalPlayerController) == false || IsValid(ViewTarget) == false || ViewTarget != MyTarget)
	{
		return false;
	}

	AMHudInGame* Hud = LocalPlayerController->GetHUD<AMHudInGame>();
	if (IsValid(Hud) == false)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ViewTarget->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilitySystemComponent) == false)
	{
		return false;
	}

	Hud->Test(AbilitySystemComponent, Parameters.AggregatedSourceTags.First());

	return true;
}

bool UGameplayCue_SkillCoolDown::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (IsValid(MyTarget) == false)
	{
		return false;
	}

	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn* ViewTarget = Cast<APawn>(LocalPlayerController->GetViewTarget());
	if (IsValid(LocalPlayerController) == false || IsValid(ViewTarget) == false || ViewTarget != MyTarget)
	{
		return false;
	}

	AMHudInGame* Hud = LocalPlayerController->GetHUD<AMHudInGame>();
	if (IsValid(Hud) == false)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ViewTarget->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilitySystemComponent) == false)
	{
		return false;
	}

	FGameplayTagContainer AbilityTags = Parameters.AggregatedSourceTags.Filter(FGameplayTag::RequestGameplayTag("Ability").GetSingleTagContainer());
	if (AbilityTags.IsEmpty())
	{
		return false;
	}

	Hud->Test(AbilitySystemComponent, AbilityTags.GetByIndex(0));

	return true;
}

bool UGamepalyCue_Freeze::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	return true;
}

bool UGamepalyCue_Freeze::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(MyTarget));
	if (IsValid(GameState) == false)
	{
		return false;
	}

	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(LocalPlayerController) == false)
	{
		return false;
	}

	APawn* ViewTarget = Cast<APawn>(LocalPlayerController->GetViewTarget());
	if (IsValid(ViewTarget) == false || ViewTarget != MyTarget)
	{
		return false;
	}

	GameState->UnregistMPCParam("Test");

	return true;
}

bool UGamepalyCue_Freeze::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(MyTarget));
	if (IsValid(GameState) == false)
	{
		return false;
	}

	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(LocalPlayerController) == false)
	{
		return false;
	}

	APawn* ViewTarget = Cast<APawn>(LocalPlayerController->GetViewTarget());
	if (IsValid(ViewTarget) == false || ViewTarget != MyTarget)
	{
		return false;
	}

	GameState->RegistMPCParam("Test");

	return true;
}
