#include "Bullet.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
 
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"
#include "TestGame/MAbility/MEffect.h"

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
}

void ABullet::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsReactable(OtherActor) == false)
	{
		return;
	}

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

void ADamageGiveActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsReactable(OtherActor) && HasAuthority() && IsValid(Owner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Owner->GetComponentByClass<UAbilitySystemComponent>())
		{
			FGameplayEffectSpecHandle GameplayEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UGameplayEffect_Damage::StaticClass(), -1, AbilitySystemComponent->MakeEffectContext());
			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*GameplayEffectSpecHandle.Data.Get(), OtherActor->GetComponentByClass<UAbilitySystemComponent>());

			// 넉백 처리, 임시로 한거고 추후에 넉백저항 등이 들어가면 어트리뷰트에 기반해야 할 듯?
			FVector Offset = FVector::ZeroVector;
			if (UCapsuleComponent* CapsuleComponent = Owner->GetComponentByClass<UCapsuleComponent>())
			{
				Offset.Z = CapsuleComponent->GetScaledCapsuleHalfHeight();
			}
			if (UCharacterMovementComponent* MovementComponent = OtherActor->GetComponentByClass<UCharacterMovementComponent>())
			{
				MovementComponent->StopMovementImmediately();
				MovementComponent->AddRadialImpulse(Owner->GetActorLocation() + Offset, 1000.f, 150000.f, ERadialImpulseFalloff::RIF_Linear, false);
			}
		}
	}
}

void ADamageGiveActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	IgnoreActors.Add(GetOwner());
	IgnoreActors.Add(GetInstigator());
}

bool ADamageGiveActor::IsReactable(AActor* InActor)
{
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
	}

	return true;
}