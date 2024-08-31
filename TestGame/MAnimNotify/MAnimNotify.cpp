#include "MAnimNotify.h"
#include "TestGame/Bullet/Bullet.h"
#include "TestGame/MCharacter/MCharacter.h"

void UMAnimNotify_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UWorld* World = MeshComp->GetWorld();

	if (IsValid(ActorClass) && IsValid(World))
	{
		FTransform SpawnTransform;
		if (SpawnSocketName != NAME_None)
		{
			SpawnTransform = MeshComp->GetSocketTransform(SpawnSocketName);
		}
		else if(AActor* Actor = MeshComp->GetOwner())
		{
			SpawnTransform = Actor->GetActorTransform();
		}
		SpawnTransform.SetRotation(FRotator::ZeroRotator.Quaternion());
		SpawnTransform.AddToTranslation(SpawnOffset);
		SpawnActor = World->SpawnActor<AActor>((ActorClass), SpawnTransform);
	}
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
		Rotator.Yaw = Character->GetTargetAngle();
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
