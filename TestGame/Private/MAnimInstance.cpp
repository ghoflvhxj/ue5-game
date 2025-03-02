#include "MAnimInstance.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/MMonster.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"

void UMAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

#if WITH_EDITOR
	if (GIsEditor && IsValid(Actions))
	{
		for (FMActionInfo ActionInfo : Actions->ActionInfos)
		{
			Animations.FindOrAdd(ActionInfo.ActionTag) = ActionInfo.ActionMontage.Get();
		}
	}
#endif
}


void UMAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// 에디터 상에서는 폰이 세팅이 안되있음.
	// 에디터에서 보고 싶다면... 에디터 전용 무언가를 세팅해줘야 할듯
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (UMActionComponent* ActionComponent = TryGetPawnOwner()->GetComponentByClass<UMActionComponent>())
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this, ActionComponent]() {
				if (IsValid(ActionComponent))
				{
					ActionComponent->CopyActions(Animations);
				}
			});
			//ActionComponent->CopyActions(Animations);
		}
	}
}

void UMAnimInstance::AnimNotify_Combo()
{
	if (AMCharacter* Character = Cast<AMCharacter>(TryGetPawnOwner()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			Weapon->OnAttackCoolDownFinished();
		}
	}
}

void UMAnimInstance::AnimNotify_Ragdoll()
{
	if (AMCharacter* Character = Cast<AMCharacter>(TryGetPawnOwner()))
	{
		Character->Ragdoll();
	}
}

//void UMonsterAnimInstnace::NativeInitializeAnimation()
//{
//	Super::NativeInitializeAnimation();
//
//#if WITH_EDITOR
//	if (GIsEditor && IsValid(MonsterTable))
//	{
//		if (FMonsterTableRow* MonsterTableRow = MonsterTable->FindRow<FMonsterTableRow>(*FString::FromInt(MonsterIndex), TEXT("MonsterTable")))
//		{
//			if (UClass* MonsterClass = MonsterTableRow->MonsterData.MonsterClass.TryLoadClass<AMMonster>())
//			{
//				if (AMMonster* Monster = MonsterClass->GetDefaultObject<AMMonster>())
//				{
//					SetMesh(Monster->GetMesh());
//				}
//			}
//		}
//	}
//#endif
//}
