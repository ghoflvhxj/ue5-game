#include "MAnimNotify.h"
#include "TestGame/Bullet/Bullet.h"

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

	FVector Direction = MeshComp->GetForwardVector();
	float Damage = 0.f;
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;

	if (AActor* Actor = MeshComp->GetOwner())
	{
		Direction = Actor->GetActorForwardVector();
		AbilitySystemComponent = Actor->GetComponentByClass<UAbilitySystemComponent>();
	}

	if (ABullet* Bullet = GetSpawnActor<ABullet>())
	{
		Bullet->GiveEffects(AbilitySystemComponent);
		Bullet->StartProjectile(Direction, 0.f);
	}
}
