#include "DamageComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/GameStateBase.h"

#include "TestGame/MCharacter/Component/MBattleComponent.h"

#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MCharacter/MCharacter.h"

UMDamageComponent::UMDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	IgnoreActors.Add(GetOwner()->GetInstigator());
	IgnoreActors.Add(GetOwner()->GetOwner());

	if (bApplyDamageOnOverlap)
	{
		GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UMDamageComponent::TryGiveDamage);
		GetOwner()->OnActorEndOverlap.AddDynamic(this, &UMDamageComponent::DiscardTarget);
	}

	OwnerCapsule = GetOwner()->GetComponentByClass<UCapsuleComponent>();

	if (IsValid(DamageEffectClass) == false)
	{
		DamageEffectClass = UGameplayEffect_Damage::StaticClass();
	}

	AActor* TempInstigator = GetOwner();
	while (IsValid(TempInstigator))
	{
		APawn* NewInst = TempInstigator->GetInstigator();
		if (IsValid(NewInst) && NewInst != TempInstigator)
		{
			TempInstigator = NewInst;
		}
		else
		{
			break;
		}
	}

	DamageInstigator = Cast<APawn>(TempInstigator);
}

void UMDamageComponent::TryGiveDamage(AActor* OverlappedActor, AActor* OtherActor)
{
	GiveDamage(OtherActor);
}

bool UMDamageComponent::GiveDamage(AActor* OtherActor)
{
	if (IsReactable(OtherActor))
	{
		React(OtherActor);
		return true;
	}

	return false;
}

void UMDamageComponent::DiscardTarget(AActor* OverlappedActor, AActor* OtherActor)
{
	AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
	if (IsValid(GameState) == false)
	{
		return;
	}

	MapTargetToLastDamageTime.Remove(OtherActor);

	// 멀티플레이 환경에서 대미지가 한번에 여러 번 들어가는 걸 방지하기 위함
	// 주기가 없이 한번만 때리는 것은 홀딩하고
	// 주기가 있다면 주기에 맡김 
	if (Period == 0.f)
	{
		MapTargetToDamageHold.Add(OtherActor, GameState->GetServerWorldTimeSeconds());
	}
}

void UMDamageComponent::React(AActor* InActor)
{
	AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
	if (IsValid(GameState) == false)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor) == false)
	{
		return;
	}

	// 서버에서만 GAS 관련 처리를 함. 반응성을 위해 클라에서 따로 생성한 경우도 있어 HasAuthority는 안됨
	if (IsNetMode(NM_Client) == false)
	{
		//// 넉백 처리, 임시로 한거고 추후에 넉백저항 등이 들어가면 어트리뷰트에 기반해야 할 듯?
		//FVector Offset = FVector::ZeroVector;
		//if (UCapsuleComponent* CapsuleComponent = DamageInstigator->GetComponentByClass<UCapsuleComponent>())
		//{
		//	Offset.Z = CapsuleComponent->GetScaledCapsuleHalfHeight();
		//}
		//if (UCharacterMovementComponent* MovementComponent = InActor->GetComponentByClass<UCharacterMovementComponent>())
		//{
		//	MovementComponent->StopMovementImmediately();
		//	MovementComponent->AddRadialImpulse(DamageInstigator->GetActorLocation() + Offset, 1000.f, 150000.f, ERadialImpulseFalloff::RIF_Linear, false);
		//}

		MapTargetToLastDamageTime.FindOrAdd(InActor) = GameState->GetServerWorldTimeSeconds();

		++MapTargetToDamageCount.FindOrAdd(InActor);

		// 주기가 0이면 바로 홀딩 대상으로 인식
		if (Period == 0.f)
		{
			MapTargetToDamageHold.FindOrAdd(InActor) = GameState->GetServerWorldTimeSeconds();
		}
	}

	GetOnDamageEvent().Broadcast(OwnerActor, InActor);
}

bool UMDamageComponent::IsReactable(AActor* InActor)
{
	AGameStateBase* GameState = UGameplayStatics::GetGameState(this);

	if (IsValid(GameState) == false)
	{
		return false;
	}

	if (bDamagable == false)
	{
		return false;
	}

	if (Target != nullptr)
	{
		if (Target != InActor)
		{
			return false;
		}
	}

	if (DamageInstigator.IsValid() == false)
	{
		return false;
	}

	if (IsValid(InActor) == false || IgnoreActors.Contains(InActor))
	{
		return false;
	}

	if (AMCharacter* Character = Cast<AMCharacter>(InActor))
	{
		if (Character->IsDead())
		{
			return false;
		}

		if (UMTeamComponent* TeamComponent = Character->GetTeamComponent())
		{
			if (TeamComponent->IsSameTeam(GetOwner()->GetInstigator()))
			{
				return false;
			}
		}
	}
	else
	{
		if (InActor->ActorHasTag("Damagable") == false)
		{
			return false;
		}
	}

	if (GameState->GetServerWorldTimeSeconds() - MapTargetToLastDamageTime.FindOrAdd(InActor) < Period)
	{
		return false;
	}

	if (MapTargetToDamageHold.Contains(InActor))
	{
		return false;
	}

	if (DamageApplyMaxCount > 0 && MapTargetToDamageCount.FindOrAdd(InActor) >= DamageApplyMaxCount)
	{
		return false;
	}

	return true;
}

void UMDamageComponent::Reset()
{
	MapTargetToDamageCount.Empty();
	MapTargetToLastDamageTime.Empty();
	MapTargetToDamageHold.Empty();

	bDamagable = true;
}

void UMDamageComponent::SetDamageParams(const TMap<FGameplayTag, float>& InParams)
{
	for (auto ParamIter = InParams.CreateConstIterator(); ParamIter; ++ParamIter)
	{
		const FGameplayTag& Tag = ParamIter->Key;
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("DamageParam")))
		{
			DamageParams.FindOrAdd(Tag) = ParamIter->Value;
		}
	}
}

void UMDamageComponent::AddGameplayEffect(TSubclassOf<UGameplayEffect> InEffectClass)
{
	Effects.Add(InEffectClass);
}

void UMDamageComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
	if (IsValid(GameState) == false)
	{
		return;
	}

	// 홀딩 제거 로직
	if (OwnerCapsule.IsValid())
	{
		for (auto Iterator = MapTargetToDamageHold.CreateIterator(); Iterator; ++Iterator)
		{
			AActor* Actor = Iterator.Key();
			if (IsValid(Actor) == false)
			{
				Iterator.RemoveCurrent();
				continue;
			}

			if (UCapsuleComponent* DamagedCapsule = Actor->GetComponentByClass<UCapsuleComponent>())
			{
				// 거리 검사
				if (GetOwner()->GetHorizontalDistanceTo(Actor) > OwnerCapsule->GetScaledCapsuleRadius() + DamagedCapsule->GetScaledCapsuleRadius() + HoldDistance)
				{
					Iterator.RemoveCurrent();
					continue;
				}
			}
		}
	}

	for (auto& Pair : MapTargetToLastDamageTime)
	{
		GiveDamage(Pair.Key);
	}
}