#include "MMonster.h"

#include "BehaviorTree/BehaviorTreeComponent.h"

#include "MGameInstance.h"
#include "TestGame/MAbility/MAbilitySystemComponent.h"
#include "TestGame/MAbility/MActionAbility.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MActionStruct.h"
#include "TestGame/MCharacter/Component/ActionComponent.h"
#include "TestGame/Bullet/Bullet.h"

const FMonsterTableRow FMonsterTableRow::Empty = FMonsterTableRow();

void AMMonster::BeginPlay()
{
	Super::BeginPlay();

	UMGameInstance* GameInstance = Cast<UMGameInstance>(GetGameInstance());
	if (HasAuthority() && IsValid(GameInstance))
	{
		FMonsterTableRow MonsterTableRow = GetMonsterTableRow();
		for (int32 ActionIndex : MonsterTableRow.MonsterData.ActionIndices)
		{
			const FActionTableRow& ActionTableRow = GameInstance->GetActionTableRow(ActionIndex);
			//FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ActionTableRow.AbilityClass));
			
			if (const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(ActionTableRow.AbilityClass))
			{
				if (UGameplayAbility_Skill* SkillAbility = Cast<UGameplayAbility_Skill>(AbilitySpec->GetPrimaryInstance()))
				{
					SkillAbility->SetSkillIndex(ActionTableRow.SkillIndex, FGameplayTag::EmptyTag);
				}
			}
		}

		if (UDataTable* AttributeData = Cast<UDataTable>(MonsterTableRow.MonsterData.AttributeData.TryLoad()))
		{
			AbilitySystemComponent->InitStats(UMAttributeSet::StaticClass(), AttributeData);
		}
	}

}

void AMMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMonster, MonsterIndex, COND_InitialOnly);
}

void AMMonster::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
}

void AMMonster::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//if (MyComp == GetMesh())
	//{
	//	if (UMDamageComponent* DamageComponent = GetComponentByClass<UMDamageComponent>())
	//	{
	//		if (DamageComponent->IsReactable(Other))
	//		{
	//			DamageComponent->React(Other);
	//		}
	//	}
	//}
}

void AMMonster::Freeze(const FGameplayTag InTag, int32 InStack)
{
	Super::Freeze(InTag, InStack);

	if (InStack > 0)
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
		{
			SkeletalMeshComponent->bNoSkeletonUpdate = true;
			SkeletalMeshComponent->bPauseAnims = true;
		}

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->RotationRate = FRotator::ZeroRotator;
		}

		if (AController* MyController = GetController())
		{
			if (UBehaviorTreeComponent* BrainComponent = MyController->GetComponentByClass<UBehaviorTreeComponent>())
			{
				BrainComponent->PauseLogic(TEXT("Pause"));
			}
			//MyController->StopMovement();
		}
	}
	else
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
		{
			SkeletalMeshComponent->bNoSkeletonUpdate = false;
			SkeletalMeshComponent->bPauseAnims = false;
		}

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->RotationRate = FRotator(0.0, 360.0, 0.0);
		}

		if (AController* MyController = GetController())
		{
			if (UBehaviorTreeComponent* BrainComponent = MyController->GetComponentByClass<UBehaviorTreeComponent>())
			{
				BrainComponent->ResumeLogic(TEXT("Pause"));
			}
			//MyController->Movement();
		}
	}
}

int32 AMMonster::GetEffectIndex(const FGameplayTag& InTag) const
{
	const FMonsterTableRow& MonsterTableRow = GetMonsterTableRow();
	return MonsterTableRow.MonsterData.MapTagToEffectIndex.Contains(InTag) ? MonsterTableRow.MonsterData.MapTagToEffectIndex[InTag] : INDEX_NONE;
}

int32 AMMonster::GetDropIndex_Implementation()
{
	const FMonsterTableRow& MonsterTableRow = GetMonsterTableRow();
	return MonsterTableRow.MonsterData.DropIndex;
}

const FMonsterTableRow& AMMonster::GetMonsterTableRow() const
{
	if (UMGameInstance* GameInstance = Cast<UMGameInstance>(GetGameInstance()))
	{
		return GameInstance->GetMonsterTableRow(MonsterIndex);
	}

	return FMonsterTableRow::Empty;
}

const FActionTableRow& AMMonster::GetActionTableRow() const
{
	if (UMGameInstance* GameInstance = Cast<UMGameInstance>(GetGameInstance()))
	{
		return GameInstance->GetActionTableRow(LoadedAction);
	}

	return FActionTableRow::Empty;
}

void AMMonster::SetWeaponCollisionEnable_Implementation(bool bInEnable)
{
	if (UPrimitiveComponent* WeaponCollision = GetWeaponCollision())
	{
		WeaponCollision->SetGenerateOverlapEvents(bInEnable);
	}
}

UPrimitiveComponent* AMMonster::GetWeaponCollision()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		TArray<USceneComponent*> ChildComponents;
		MeshComponent->GetChildrenComponents(false, ChildComponents);

		for (USceneComponent* ChildComponent : ChildComponents)
		{
			if (ChildComponent->IsA<UShapeComponent>())
			{
				return Cast<UPrimitiveComponent>(ChildComponent);
			}
		}
	}

	return nullptr;
}

int32 AMMonster::GetLoadedAction()
{
	return LoadedAction;
}

TArray<int32> AMMonster::GetActions()
{
	FMonsterTableRow MonsterTableRow = GetMonsterTableRow();
	TArray<int32> Actions;
	for (int32 ActionIndex : MonsterTableRow.MonsterData.ActionIndices)
	{
		Actions.Add(ActionIndex);
	}

	return Actions;
}

void AMMonster::LoadAction()
{
	//FMonsterTableRow MonsterTableRow = GetMonsterTableRow();
	//TArray<int32> Actions;
	//for (int32 ActionIndex : MonsterTableRow.MonsterData.ActionIndices)
	//{
	//	Actions.Add(ActionIndex);
	//}

	//if (Actions.IsEmpty() == false)
	//{
	//	LoadedAction = Actions[FMath::Rand() % Actions.Num()];
	//}
}

void AMMonster::SetAction(int32 InActionIndex)
{
	LoadedAction = InActionIndex;
}
