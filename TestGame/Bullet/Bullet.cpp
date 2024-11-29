#include "Bullet.h"

#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"

DECLARE_LOG_CATEGORY_CLASS(LogBullet, Log, Log)

// Sets default values
ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetComponentTickEnabled(false);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SphereComponent);
	StaticMeshComponent->SetComponentTickEnabled(false);

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilityComponent"));

	//PrimaryActorTick.bCanEverTick = false;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ProjectileComponent))
	{
		ProjectileComponent->Deactivate();
	}

	if (HasAuthority())
	{
		if (IsValid(Owner))
		{
			if (UAbilitySystemComponent* OwnerAbilitySystemComponent = Owner->GetComponentByClass<UAbilitySystemComponent>())
			{
				Damage = OwnerAbilitySystemComponent->GetNumericAttribute(UMWeaponAttributeSet::GetAttackPowerAttribute());
			}
		}

		if (IsValid(AbilitySystemComponent))
		{
			FGameplayAbilitySpec NewSpec(UGameplayAbility_CollideDamage::StaticClass());
			FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(NewSpec);
			if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecHandle))
			{
				for(UGameplayAbility* AbilityInstance : Spec->GetAbilityInstances())
				{
					if (UGameplayAbility_CollideDamage* CollideDamageAbility = Cast<UGameplayAbility_CollideDamage>(AbilityInstance))
					{
						CollideDamageAbility->SetDamage(Damage);
					}
				}

			}

			AbilitySystemComponent->TryActivateAbility(SpecHandle);
		}
	}


	IgnoreActors.Add(GetInstigator());
}

void ABullet::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsReactable(OtherActor) == false)
	{
		return;
	}

	//if (HasAuthority())
	//{
	//	if (UAbilitySystemComponent* AbilitySystemComponent = OtherActor->FindComponentByClass<UAbilitySystemComponent>())
	//	{
	//		UE_LOG(LogBullet, Log, TEXT("%p, %s Apply Effect Num:%d"), this, *GetName(), GameplayEffects.Num());

	//		for (auto GameplayEffect : GameplayEffects)
	//		{
	//			FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	//			EffectContextHandle.AddSourceObject(this);
	//			EffectContextHandle.AddInstigator(GetInstigator()/*Character*/, GetOwner()/*Weapon*/);

	//			AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, 1, EffectContextHandle);
	//		}
	//	}
	//}

	if (bPenerate == false)
	{
		if (HasAuthority())
		{
			if (IsValid(ProjectileComponent))
			{
				ProjectileComponent->StopMovementImmediately();
			}
			SetLifeSpan(1.f);
		}

		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
	
}

//void ABullet::GiveEffects(UAbilitySystemComponent* AbilitySystemComponent)
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//
//	GameplayEffects.Reset();
//
//	if (IsValid(AbilitySystemComponent))
//	{
//		Damage = AbilitySystemComponent->GetNumericAttribute(UMAttributeSet::GetAttackPowerAttribute());
//	}
//
//	// 임시 작업
//	// 이후에 최대 체력 감소, 마나 감소, 시전 시간 증가 등 다양한 이펙트를 추가할 수 있는 구조로 변경
//	// 그리고 Effects를 소유할 수 있는 컨테이너 컴포넌트를 만들자
//	{
//		UGameplayEffect* NewHealthAddEffect = NewObject<UGameplayEffect>();
//		NewHealthAddEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
//
//		FGameplayModifierInfo ModifierInfo;
//		ModifierInfo.Attribute = UMAttributeSet::GetHealthAttribute();
//		ModifierInfo.ModifierOp = EGameplayModOp::Additive;
//		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Damage);
//		NewHealthAddEffect->Modifiers.Add(ModifierInfo);
//
//		GameplayEffects.Add(NewHealthAddEffect);
//	}
//}

void ABullet::DestroyBullet()
{
	SetActorHiddenInGame(true);
	SetLifeSpan(0.1f);
}

void ABullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == false)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			SetActorTickEnabled(GetDistanceTo(PlayerController->GetViewTarget()) < 2000.f);
		}
	}
}	

void ABullet::StartProjectile(const FVector& NewDirection, float NewDamage)
{
	Direction = NewDirection;
	//Damage = NewDamage;

	ProjectileComponent->Activate();
	ProjectileComponent->Velocity = Direction * ProjectileComponent->InitialSpeed;

	SetLifeSpan(10.f);
}

bool ABullet::IsReactable(AActor* InActor)
{
	if (IsValid(InActor) == false || IgnoreActors.Contains(InActor))
	{
		return false;
	}

	if (InActor->IsA<APawn>() == false)
	{
		return false;
	}

	if (AMCharacter* Character = Cast<AMCharacter>(InActor))
	{
		if (Character->IsDead())
		{
			return false;
		}
	}

	return true;
}