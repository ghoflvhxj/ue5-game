#include "MAnimNotify.h"
#include "TestGame/Bullet/Bullet.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"

DECLARE_LOG_CATEGORY_CLASS(LogNotify, Log, Log)

void UMAnimNotify_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UWorld* World = MeshComp->GetWorld();

	if (IsValid(ActorClass) && IsValid(World) && IsValid(World->GetAuthGameMode()))
	{
		FTransform SpawnTransform = GetSocketTransform(MeshComp);
		SpawnTransform.SetRotation(FRotator::ZeroRotator.Quaternion());
		SpawnTransform.AddToTranslation(SpawnOffset);

		//FActorSpawnParameters SpawnParam;
		//GetSpawnParam(MeshComp, SpawnParam);
		UObject* ContextObject = GetContextObject(MeshComp);

		if (AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(ContextObject == nullptr ? World : ContextObject, ActorClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			OnSpawn(NewActor, MeshComp);
			UGameplayStatics::FinishSpawningActor(NewActor, SpawnTransform);
			OnSpawnFinished(NewActor, MeshComp);
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

AActor* UMAnimNotify_SpawnActor::GetContextObject(USkeletalMeshComponent* MeshComp)
{
	return nullptr;
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

void UMAnimNotify_SpawnBullet::OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(MeshComp) == false || IsValid(Owner) == false)
	{
		return;
	}

	if (ABullet* Bullet = Cast<ABullet>(InActor))
	{
		if (IsValid(Owner))
		{
			Bullet->SetOwner(Owner);
			if (UAbilitySystemComponent* AbilitySystemComponent = Owner->GetComponentByClass<UAbilitySystemComponent>())
			{
				//Bullet->GiveEffects(AbilitySystemComponent);
			}
		}
	}
}

void UMAnimNotify_SpawnBullet::OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(MeshComp) == false || IsValid(Owner) == false)
	{
		return;
	}

	if (ABullet* Bullet = Cast<ABullet>(InActor))
	{
		FVector Direction = MeshComp->GetForwardVector();
		if (AMCharacter* Character = Cast<AMCharacter>(Owner))
		{
			FRotator Rotator = FRotator::ZeroRotator;
			//Rotator.Yaw = Character->GetTargetAngle();
			Rotator.Yaw = Character->GetActorRotation().Yaw;
			Direction = Rotator.Vector();
		}

		float Damage = 0.f;
		Bullet->StartProjectile(Direction, Damage);
	}
}

bool UMAnimNotify_SpawnBullet::GetSpawnParam(USkeletalMeshComponent* MeshComp, FActorSpawnParameters& OutSpawnTransform)
{
	if (Super::GetSpawnParam(MeshComp, OutSpawnTransform))
	{
		if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
		{
			OutSpawnTransform.Owner = Character->GetEquipItem<AActor>();
			return true;
		}
	}

	return false;
}

AActor* UMAnimNotify_SpawnBullet::GetContextObject(USkeletalMeshComponent* MeshComp)
{
	if (IsValid(MeshComp))
	{
		return MeshComp->GetOwner();
	}

	return nullptr;
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
