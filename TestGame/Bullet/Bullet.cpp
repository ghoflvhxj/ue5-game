#include "Bullet.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h"

#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/MBattleComponent.h" // TeamComponent
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MAbility/MAbility.h"
#include "TestGame/MComponents/DamageComponent.h"

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

	DamageComponent = CreateDefaultSubobject<UMDamageComponent>(TEXT("DamageComponent"));

	//PrimaryActorTick.bCanEverTick = false;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	ProjectileComponent->Deactivate();
	BulletHitHandleDelegate = DamageComponent->GetOnDamageEvent().AddUObject(this, &ABullet::OnBulletHit);
}

void ABullet::OnBulletHit(AActor* InHitCauser, AActor* InActor)
{
	if (bPenerate == false)
	{
		if (IsValid(ProjectileComponent))
		{
			ProjectileComponent->StopMovementImmediately();
		}
		SetLifeSpan(1.f);

		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}

	if (bExplosion)
	{
		DamageComponent->GetOnDamageEvent().Remove(BulletHitHandleDelegate);

		if (IsNetMode(NM_Client) == false)
		{
			TArray<FOverlapResult> OverlapResults;
			FCollisionObjectQueryParams CollisionObjectQueryParam;
			CollisionObjectQueryParam.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

			FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ExplosionRadius);
			GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, CollisionObjectQueryParam, CollisionShape);

			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				DamageComponent->GiveDamage(OverlapResult.GetActor());
			}
		}
		
		if (IsNetMode(NM_DedicatedServer) == false)
		{
			FVector Location = GetActorLocation();
			if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(GetActorLocation(), NavLocation))
				{
					Location = NavLocation.Location;
				}
			}

			FTransform FXTransform;
			FXTransform.SetLocation(Location);

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Cast<UParticleSystem>(FXAsset), FXTransform, true, EPSCPoolMethod::AutoRelease);
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, Location);
		}
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

	SphereComponent->SetWorldRotation(NewDirection.ToOrientationRotator());

	SetLifeSpan(10.f);
}

void ABullet::StartProjectile()
{
	StartProjectile(GetActorForwardVector(), 0.f);
}

void ADamageGiveActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsReactable(OtherActor))
	{
		React(OtherActor);
	}
}

void ADamageGiveActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	IgnoreActors.Add(GetOwner());
	IgnoreActors.Add(GetInstigator());
}

void ADamageGiveActor::React(AActor* InActor)
{
	APawn* DamageInstigator = GetInstigator();

	if (IsNetMode(NM_Client) == false && IsValid(DamageInstigator))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = DamageInstigator->GetComponentByClass<UAbilitySystemComponent>())
		{
			//FGameplayEffectSpecHandle GameplayEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UGameplayEffect_Damage::StaticClass(), -1, AbilitySystemComponent->MakeEffectContext());
			//AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*GameplayEffectSpecHandle.Data.Get(), InActor->GetComponentByClass<UAbilitySystemComponent>());

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
		}
	}
}

bool ADamageGiveActor::IsReactable(AActor* InActor)
{
	if (bDamagable == false)
	{
		return false;
	}

	if (IsValid(InActor) == false || IgnoreActors.Contains(InActor))
	{
		return false;
	}

	if (IsValid(GetOwner()) == false || IsValid(GetOwner()->GetInstigator()) == false)
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
			if (TeamComponent->IsSameTeam(GetInstigator()))
			{
				return false;
			}
		}
	}

	return true;
}