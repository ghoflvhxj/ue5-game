#include "MAnimNotify.h"
#include "TestGame/Bullet/Bullet.h"
#include "TestGame/MCharacter/MCharacter.h"

void UMAnimNotify_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UWorld* World = MeshComp->GetWorld();

	if (IsValid(ActorClass) && IsValid(World))
	{
		FTransform SpawnTransform = GetSocketTransform(MeshComp);
		SpawnTransform.SetRotation(FRotator::ZeroRotator.Quaternion());
		SpawnTransform.AddToTranslation(SpawnOffset);

		FActorSpawnParameters SpawnParam;
		GetSpawnParam(MeshComp, SpawnParam);

		if (AActor* NewActor = World->SpawnActor<AActor>((ActorClass), SpawnTransform, SpawnParam))
		{
			OnActorSpawned(NewActor, MeshComp);
		}
	}
}

bool UMAnimNotify_SpawnActor::GetSpawnParam(USkeletalMeshComponent* MeshComp, FActorSpawnParameters& OutSpawnTransform)
{
	if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
	{
		OutSpawnTransform.Instigator = Character;
		return true;
	}

	return false;
}

FTransform UMAnimNotify_SpawnActor::GetSocketTransform(USkeletalMeshComponent* MeshComp)
{
	FTransform Transform;
	if (SpawnSocketName != NAME_None)
	{
		Transform = MeshComp->GetSocketTransform(SpawnSocketName);
	}
	else if (AActor* Actor = MeshComp->GetOwner())
	{
		Transform = Actor->GetActorTransform();
	}

	return Transform;
}

void UMAnimNotify_SpawnBullet::OnActorSpawned(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(MeshComp) == false || IsValid(Owner) == false)
	{
		return;
	}

	FVector Direction = MeshComp->GetForwardVector();
	float Damage = 0.f;


	if (AMCharacter* Character = Cast<AMCharacter>(Owner))
	{
		FRotator Rotator = FRotator::ZeroRotator;
		//Rotator.Yaw = Character->GetTargetAngle();
		Rotator.Yaw = Character->GetActorRotation().Yaw;
		Direction = Rotator.Vector();
	}

	if (ABullet* Bullet = Cast<ABullet>(InActor))
	{
		if (IsValid(Owner))
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = Owner->GetComponentByClass<UAbilitySystemComponent>())
			{
				Bullet->GiveEffects(AbilitySystemComponent);
			}
		}

		Bullet->StartProjectile(Direction, 0.f);
	}
}

bool UMAnimNotify_SpawnBullet::GetSpawnParam(USkeletalMeshComponent* MeshComp, FActorSpawnParameters& OutSpawnTransform)
{
	if (Super::GetSpawnParam(MeshComp, OutSpawnTransform))
	{
		if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
		{
			OutSpawnTransform.Owner = Character->GetWeapon<AActor>();
			return true;
		}
	}

	return false;
}

FTransform UMAnimNotify_SpawnBullet::GetSocketTransform(USkeletalMeshComponent* MeshComp)
{
	FTransform Transform;

	if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
	{
		if (Character->GetWeaponMuzzleTransform(Transform))
		{
			return Transform;
		}
	}

	return Super::GetSocketTransform(MeshComp);
}
