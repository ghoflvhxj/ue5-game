#include "DamageComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/GameStateBase.h"

#include "TestGame/MCharacter/Component/MBattleComponent.h"

#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MCharacter/MCharacter.h"

void UMDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	IgnoreActors.Add(GetOwner()->GetInstigator());
	IgnoreActors.Add(GetOwner()->GetOwner());

	if (bApplyDamageOnOverlap)
	{
		GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UMDamageComponent::Overlap);
	}

	OwnerCapsule = GetOwner()->GetComponentByClass<UCapsuleComponent>();
	if (OwnerCapsule.IsValid())
	{
		GetOwner()->GetWorldTimerManager().SetTimer(DamageHoldUpdateTimerHandle, FTimerDelegate::CreateUObject(this, &UMDamageComponent::UpdateHold), 0.1f, true);
	}

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

void UMDamageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetOwner()->GetWorldTimerManager().ClearTimer(DamageHoldUpdateTimerHandle);
	DamageHoldUpdateTimerHandle.Invalidate();
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

void UMDamageComponent::UpdateHold()
{
	for (auto Iterator = DamageHold.CreateIterator(); Iterator; ++Iterator)
	{
		if (IsValid(*Iterator) == false)
		{
			Iterator.RemoveCurrent();
			continue;
		}

		if (UCapsuleComponent* DamagedCapsule = (*Iterator)->GetComponentByClass<UCapsuleComponent>())
		{
			if (GetOwner()->GetDistanceTo(*Iterator) > OwnerCapsule->GetScaledCapsuleRadius() + DamagedCapsule->GetScaledCapsuleRadius() + HoldDistance)
			{
				Iterator.RemoveCurrent();
				continue;
			}
		}
	}
}

void UMDamageComponent::Overlap(AActor* OverlappedActor, AActor* OtherActor)
{
	GiveDamage(OtherActor);
}

void UMDamageComponent::React(AActor* InActor)
{
	AActor* OwnerActor = GetOwner();

	// 아래 로직들 굳이 여기서 할 필요가 있을까?
	// ApplyEffect 문은 어빌리티에서 GetDamageApplideEvent를 가져와 바인딩 시켜버리면 좋을듯?

	// 서버에서만 GAS 관련 처리를 함. 반응성을 위해 클라에서 따로 생성한 경우도 있어 HasAuthority는 안됨
	if (IsNetMode(NM_Client) == false)
	{
		UAbilitySystemComponent* InstigatorASC = DamageInstigator->GetComponentByClass<UAbilitySystemComponent>();
		UAbilitySystemComponent* OtherASC = InActor->GetComponentByClass<UAbilitySystemComponent>();
		if (IsValid(InstigatorASC) && IsValid(OtherASC))
		{
			// 대미지 테이블을 만들자.  이펙트, 넉백, 공격% 등이 설정될 수 있도록

			//// 히트 이펙트 로직. 특정 위치에 표시하려면 CueParam의 Location을 채우고 사용하려면 직접 ExecutePlayCue를 호출해야 함
			//FGameplayCueParameters CueParams;
			//CueParams.EffectCauser = OwnerActor;
			//CueParams.Instigator = DamageInstigator;
			//CueParams.Location = InActor->GetActorLocation() + (OwnerActor->GetActorLocation() - InActor->GetActorLocation()) / 2.f;

			//FGameplayTag HitEffectCueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Effect.Hit.Default");
			//if (UGameplayEffect_Damage* GEDamgeCDO = Cast<UGameplayEffect_Damage>(DamageEffectClass->GetDefaultObject()))
			//{
			//	HitEffectCueTag = GEDamgeCDO->GetHitEffectCue();
			//}

			//OtherASC->ExecuteGameplayCue(HitEffectCueTag, CueParams);

			//// 대미지 GE 적용. DamageParam 을 설정함
			//FGameplayEffectSpecHandle DamageEffectSpecHandle = InstigatorASC->MakeOutgoingSpec(DamageEffectClass, -1, InstigatorASC->MakeEffectContext());
			//for (const TPair<FGameplayTag, float> ParamToValuePair : DamageParams)
			//{
			//	DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(ParamToValuePair.Key, ParamToValuePair.Value);
			//}
			//InstigatorASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), OtherASC);
		}

		// 넉백 처리, 임시로 한거고 추후에 넉백저항 등이 들어가면 어트리뷰트에 기반해야 할 듯?
		FVector Offset = FVector::ZeroVector;
		if (UCapsuleComponent* CapsuleComponent = DamageInstigator->GetComponentByClass<UCapsuleComponent>())
		{
			Offset.Z = CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
		if (UCharacterMovementComponent* MovementComponent = InActor->GetComponentByClass<UCharacterMovementComponent>())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->AddRadialImpulse(DamageInstigator->GetActorLocation() + Offset, 1000.f, 150000.f, ERadialImpulseFalloff::RIF_Linear, false);
		}

		if (AGameStateBase* GameState = UGameplayStatics::GetGameState(this))
		{
			DamageTimeMap.FindOrAdd(InActor) = GameState->GetServerWorldTimeSeconds();
		}

		++DamageCountMap.FindOrAdd(InActor);
		DamageHold.Add(InActor);
	}

	GetOnDamageEvent().Broadcast(OwnerActor, InActor);
}

bool UMDamageComponent::IsReactable(AActor* InActor)
{
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

	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(this))
	{
		if (GameState->GetServerWorldTimeSeconds() - DamageTimeMap.FindOrAdd(InActor) < Period)
		{
			return false;
		}

		if (DamageHold.Contains(InActor))
		{
			return false;
		}

		if (DamageApplyMaxCount > 0 && DamageCountMap.FindOrAdd(InActor) >= DamageApplyMaxCount)
		{
			return false;
		}
	}

	return true;
}

void UMDamageComponent::Reset()
{
	DamageCountMap.Empty();
	DamageTimeMap.Empty();

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
