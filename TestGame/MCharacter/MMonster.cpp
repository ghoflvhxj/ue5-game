#include "MMonster.h"

void AMMonster::BeginPlay()
{
	Super::BeginPlay();

	bool bPlayLevelStart = false;
	if (IsValid(ActionComponent))
	{
		UAnimInstance* AnimInstance = IsValid(GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
		UAnimMontage* StartMontage = ActionComponent->GetActionMontage(FGameplayTag::RequestGameplayTag("Action.Start"));
		if (IsValid(AnimInstance) && IsValid(StartMontage))
		{
			PlayAnimMontage(StartMontage);
			AnimInstance->OnMontageEnded.AddDynamic(this, &AMMonster::OnStartMontageFinished);
			bPlayLevelStart = true;
		}
	}

	if (bPlayLevelStart == false)
	{
		OnStartMontageFinished(nullptr, false);
	}
}

void AMMonster::OnStartMontageFinished(UAnimMontage* Montage, bool bInterrupted)
{
	bLevelStartFinished = true;
}

TSubclassOf<UGameplayAbility> AMMonster::GetSkill()
{
	if (IsValid(AbilitySetData))
	{
		// 일단 임시로 1번째가 스킬이라고 고정
		return AbilitySetData->Abilities.IsValidIndex(1) ? AbilitySetData->Abilities[1].GameplayAbilityClass : nullptr;
	}

	return nullptr;
}
