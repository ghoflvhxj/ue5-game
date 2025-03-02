#include "CharacterLevelSubSystem.h"
#include "Net/UnrealNetwork.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "MyPlayerState.h"
#include "MGameInstance.h"
#include "SkillSubSystem.h"
#include "TestGame/MPlayerController/MPlayerController.h"
#include "TestGame/MFunctionLibrary/MContainerFunctionLibrary.h"

DECLARE_LOG_CATEGORY_CLASS(LogLevelSubSystem, Warning, Warning);

void UCharacterLevelSubSystem::AddExperiance(AActor* InActor, int32 Exp)
{
	if (IsValid(InActor) == false) 
	{
		UE_LOG(LogLevelSubSystem, Warning, TEXT("%s Invalid actor."), *FString(__FUNCTION__));
		return;
	}

	ULevelComponent* LevelComponent = InActor->GetComponentByClass<ULevelComponent>();
	if (IsValid(LevelComponent) == false)
	{
		UE_LOG(LogLevelSubSystem, Warning, TEXT("%s invalid component."), *FString(__FUNCTION__));
		return;
	}

	LevelComponent->AddExperiance(Exp);
}

ULevelComponent::ULevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULevelComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULevelComponent, CurrentExperience);
}

void ULevelComponent::AddExperiance(int32 InValue)
{
	if (IsNetSimulating())
	{
		return;
	}

	AMPlayer* PC = Cast<AMPlayer>(GetOwner());
	if (IsValid(PC) == false)
	{
		return;
	}

	FExperienceInfo OldExpInfo = CurrentExperience;
	while (CurrentExperience.CurrentExperience + InValue >= GetNextExperiance())
	{
		InValue -= GetNextExperiance() - CurrentExperience.CurrentExperience;
		CurrentExperience.CurrentExperience = 0;
		++CurrentExperience.Level;

		// OnLevelUp
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			TArray<int32> EnhanceIndices = GameInstance->GetSkillEnhanceTableRowsByPredicate([this, PC](const FSkillEnhanceTableRow& SkillEnhanceTableRow) {
				return SkillEnhanceTableRow.CharacterIndex == PC->GetPlayerState<AMPlayerState>()->GetCharacterIndex();
			});

			UContainerFunctionLibrary::ShuffleArray(EnhanceIndices);

			if (AMPlayerControllerInGame* PlayerController = PC->GetController<AMPlayerControllerInGame>())
			{
				FSkillEnhanceData Data;
				Data.EnhanceIndices = EnhanceIndices;
				while (Data.EnhanceIndices.Num() > 3)
				{
					Data.EnhanceIndices.Pop();
				}

				if (Data.EnhanceIndices.IsEmpty())
				{
					return;
				}

				PlayerController->Client_SkillEnhance(Data);
			}
		}
	}

	CurrentExperience.CurrentExperience += InValue;
	OnRep_CurrentExperience(OldExpInfo);
}

void ULevelComponent::OnRep_CurrentExperience(const FExperienceInfo& OldExpInfo)
{
	if (OldExpInfo.Level < CurrentExperience.Level)
	{
		OnLevelUp.Broadcast(CurrentExperience.Level);
	}

	OnExperienceChangedEvent.Broadcast(CurrentExperience);
}

void ULevelComponent::Server_LevlUp_Implementation()
{
	AMPlayer* PC = Cast<AMPlayer>(GetOwner());
	UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(this));

	if (IsValid(PC) && IsValid(GameInstance))
	{
		AddExperiance(GetNextExperiance() - CurrentExperience.CurrentExperience);
	}
}
