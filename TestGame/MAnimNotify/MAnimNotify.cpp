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
		SpawnActor = World->SpawnActor<AActor>((ActorClass), SpawnTransform);
	}
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

void UMAnimNotify_SpawnBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == false)
	{
		return;
	}

	FVector Direction = MeshComp->GetForwardVector();
	float Damage = 0.f;
	
	AActor* Owner = MeshComp->GetOwner();
	if (AMCharacter* Character = Cast<AMCharacter>(Owner))
	{
		FRotator Rotator = FRotator::ZeroRotator;
		//Rotator.Yaw = Character->GetTargetAngle();
		Rotator.Yaw = Character->GetActorRotation().Yaw;
		Direction = Rotator.Vector();
	}

	if (ABullet* Bullet = GetSpawnActor<ABullet>())
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
