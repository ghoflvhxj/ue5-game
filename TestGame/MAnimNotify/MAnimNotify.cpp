#include "MAnimNotify.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "TestGame/Bullet/Bullet.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MWeapon/Weapon.h"

DECLARE_LOG_CATEGORY_CLASS(LogNotify, Log, Log)

void UMAnimNotify_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == false || IsValid(ActorClass) == false)
	{
		return;
	}

	if (UWorld* World = MeshComp->GetWorld())
	{
		if (GIsEditor == false)
		{
			if (IsValid(World->GetAuthGameMode()) == false)
			{
				return;
			}
		}

		if (IsSpawnable(World) == false)
		{
			return;
		}

		// Replicated라면 클라는 스폰 생략
		bool bShouldSpawn = true;
		if (ActorClass->GetDefaultObject<AActor>()->GetIsReplicated() && World->IsNetMode(NM_Client))
		{
			bShouldSpawn = false;
		}

		FTransform SpawnTransform = GetSocketTransform(MeshComp);
		SpawnTransform.SetRotation(FRotator::ZeroRotator.Quaternion());
		SpawnTransform.AddToTranslation(SpawnOffset);

		if (bShouldSpawn)
		{
			if (AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(World, ActorClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				OnSpawn(NewActor, MeshComp);
				UGameplayStatics::FinishSpawningActor(NewActor, SpawnTransform);
				OnSpawnFinished(NewActor, MeshComp);
			}
		}

		if (IsValid(Sound) && World->IsNetMode(NM_DedicatedServer) == false)
		{
			UGameplayStatics::PlaySoundAtLocation(MeshComp, Sound, SpawnTransform.GetLocation());
		}
	}
}

void UMAnimNotify_SpawnActor::OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	AActor* MeshOwner = MeshComp->GetOwner();

	if (AMCharacter* Character = Cast<AMCharacter>(MeshOwner))
	{
		InActor->SetOwner(bWeaponOwning ? Character->GetEquipItem<AActor>() : Character);
		InActor->SetInstigator(Character);
	}
	else
	{
		InActor->SetOwner(MeshOwner);
		APawn* Instigator = Cast<APawn>(MeshOwner);
		while (Instigator)
		{
			APawn* NewInstigator = Instigator->GetInstigator();
			if (IsValid(NewInstigator) && Instigator != NewInstigator)
			{
				Instigator = NewInstigator;
			}
			{
				break;
			}
		}
		InActor->SetInstigator(Instigator);
	}
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

bool UMAnimNotify_SpawnBullet::IsSpawnable(UWorld* World)
{
	return true;
}

void UMAnimNotify_SpawnBullet::OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	Super::OnSpawn(InActor, MeshComp);

	if (IsValid(InActor) == false)
	{
		return;
	}

	if (IsValid(MeshComp) == false)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	if (Particle)
	{
		FTransform SpawnTransform = GetSocketTransform(MeshComp);
		SpawnTransform.SetScale3D(ParticleScale);
		UParticleSystemComponent* ReturnComp = UGameplayStatics::SpawnEmitterAtLocation(MeshComp->GetWorld(), Particle, SpawnTransform);
	}

	if (ABullet* Bullet = Cast<ABullet>(InActor))
	{
		if (Owner->GetClass()->ImplementsInterface(UBulletShooterInterface::StaticClass()))
		{
			IBulletShooterInterface::Execute_InitBullet(Owner, Bullet);
		}
	}


}

void UMAnimNotify_SpawnBullet::OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp)
{
	Super::OnSpawnFinished(InActor, MeshComp);

	if (IsValid(InActor) == false)
	{
		return;
	}

	if (IsValid(MeshComp) == false)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(Owner) == false)
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
		else if (SpawnSocketName != NAME_None)
		{
			Direction = MeshComp->GetSocketLocation(SpawnSocketName) - MeshComp->GetSocketLocation("root");
			Direction.Z = 0.f;
			Direction = Direction.GetSafeNormal();
		}

		Bullet->StartProjectile(Direction, 0.f);
	}
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

void UMAnimNotifyState_WeaponActive::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
	{
		Character->SetWeaponActivation(true);
	}
}

void UMAnimNotifyState_WeaponActive::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
	{
		Character->SetWeaponActivation(false);
	}
}

void UMAnimNotifyState_TagModifier::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Actor = MeshComp->GetOwner();
	if (IsValid(Actor) == false)
	{
		return;
	}

	if (bUseReplication && Actor->HasAuthority() == false)
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(Actor, AddTags, bUseReplication);
	UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(Actor, RemoveTags, bUseReplication);

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Add Tags. %s"), *Animation->GetName());
}

void UMAnimNotifyState_TagModifier::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Actor = MeshComp->GetOwner();
	if (IsValid(Actor) == false)
	{
		return;
	}

	if (bUseReplication && Actor->HasAuthority() == false)
	{
		return;
	}

	if (bRestore)
	{
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(Actor, AddTags, bUseReplication);
		UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(Actor, RemoveTags, bUseReplication);
		UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Remove Tags"));
	}
}

void UMAnimNotifyState_AddMovemntInput::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		Character->AddMovementInput(Character->GetActorForwardVector(), Scale, bForce);
	}
}

void UAnimNotify_WeaponCharge::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMCharacter* Character = Cast<AMCharacter>(MeshComp->GetOwner()))
	{
		if (AWeapon* Weapon = Character->GetEquipItem<AWeapon>())
		{
			Weapon->Charge();
		}
	}
}

void UMAnimNotifyState_ActivateAbilityByTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	for (FName BoneName : BoneNames)
	{
		MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
	}

	Owner = MeshComp->GetOwner();
	if (Owner.IsValid())
	{
		//DamageComponent = Owner->GetComponentByClass<UMDamageComponent>();
		DrawDebugCircleArc(MeshComp->GetWorld(), Owner->GetActorLocation(), Range, Owner->GetActorForwardVector(), FMath::DegreesToRadians(Angle.X), 32, FColor::Red, false, 3.f);
	}
}

void UMAnimNotifyState_ActivateAbilityByTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	for (FName BoneName : BoneNames)
	{
		MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, false, true);
	}
}

void UMAnimNotifyState_ActivateAbilityByTag::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (Owner.IsValid() == false)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();

	//if (DamageComponent.IsValid())
	{
		//TArray<FOverlapResult> OverlapResults;
		//FCollisionObjectQueryParams ObjectQueryParams;
		//ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		//World->OverlapMultiByObjectType(OverlapResults, MeshComp->GetOwner()->GetActorLocation(), FRotator::ZeroRotator.Quaternion(), ObjectQueryParams, FCollisionShape::MakeSphere(Range));

		//for (const FOverlapResult& OverlapResult : OverlapResults)
		//{
		//	AActor* Other = OverlapResult.GetActor();

		//	FVector DirctionToOther = (Other->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
		//	float Dot = Owner->GetActorForwardVector().Dot(DirctionToOther);
		//	float AngleRadian = FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f));
		//	float AngleDegree = FMath::RadiansToDegrees(AngleRadian);

		//	if (Angle.X <= AngleDegree && AngleDegree <= Angle.Y)
		//	{
		//		DamageComponent->GiveDamage(Other);
		//	}
		//}
	}
}
