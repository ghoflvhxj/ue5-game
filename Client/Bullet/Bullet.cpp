#include "Bullet.h"

#include "Client/MCharacter/Component/MBattleComponent.h"
#include "Client/MAttribute/MAttribute.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"

// Sets default values
ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SphereComponent);

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));

	//BattleComponent = CreateDefaultSubobject<UMBattleComponent>(TEXT("BattleComponent"));

	//PrimaryActorTick.bCanEverTick = false;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ProjectileComponent))
	{
		ProjectileComponent->Deactivate();
	}
}

void ABullet::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsValid(OtherActor) == false)
	{
		return;
	}

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerPawn->GetController(), OwnerPawn, UDamageType::StaticClass());
	}

	if (OtherActor->IsA<APawn>())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = OtherActor->FindComponentByClass<UAbilitySystemComponent>())
		{
			for (auto GameplayEffect : GameplayEffects)
			{
				FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
				EffectContextHandle.AddSourceObject(this);

				AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, 1, EffectContextHandle);
			}
		}
	}
}

void ABullet::GiveEffects(UAbilitySystemComponent* AbilitySystemComponent)
{
	GameplayEffects.Reset();

	OwnerASC = AbilitySystemComponent;

	// 임시 작업
	// 이후에 최대 체력 감소, 마나 감소, 시전 시간 증가 등 다양한 이펙트를 추가할 수 있는 구조로 변경
	// 그리고 Effects를 소유할 수 있는 컨테이너 컴포넌트를 만들자
	{
		UGameplayEffect* NewHealthAddEffect = NewObject<UGameplayEffect>();
		NewHealthAddEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = FGameplayAttribute(UMAttributeSet::StaticClass()->FindPropertyByName("Health"));
		ModifierInfo.ModifierOp = EGameplayModOp::Additive;
		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Damage);
		NewHealthAddEffect->Modifiers.Add(ModifierInfo);
		GameplayEffects.Add(NewHealthAddEffect);
	}
}

void ABullet::DestroyBullet()
{
	SetActorHiddenInGame(true);
	SetLifeSpan(0.1f);
}

void ABullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABullet::StartProjectile(const FVector& NewDirection, float NewDamage)
{
	Direction = NewDirection;
	Damage = NewDamage;

	ProjectileComponent->Activate();
	ProjectileComponent->Velocity = Direction * ProjectileComponent->InitialSpeed;
}
