#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"

#include "MAnimNotify.generated.h"

class UUserWidget;

UCLASS()
class TESTGAME_API UMAnimNotify_SpawnActor : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
public:
	virtual bool IsSpawnable(UWorld* World) { return true; }
	virtual void OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp);
	virtual void OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp) {}
	virtual AActor* GetContextObject(USkeletalMeshComponent* MeshComp);
	virtual FTransform GetSocketTransform(USkeletalMeshComponent* MeshComp);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SpawnSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector SpawnOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWeaponOwning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Sound = nullptr;
};

UCLASS()
class TESTGAME_API UMAnimNotify_SpawnBullet : public UMAnimNotify_SpawnActor
{
	GENERATED_BODY()

public:
	virtual bool IsSpawnable(UWorld* World);
	virtual void OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp) override;
	virtual void OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp) override;
	virtual AActor* GetContextObject(USkeletalMeshComponent* MeshComp) override;
	virtual FTransform GetSocketTransform(USkeletalMeshComponent* MeshComp) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	TObjectPtr<UParticleSystem> Particle = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	FVector ParticleScale = FVector::OneVector;
};

UCLASS()
class TESTGAME_API UMAnimNotifyState_WeaponActive : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS()
class TESTGAME_API UMAnimNotifyState_TagModifier : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	void Restore();

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RemoveTags;
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AddTags;
	UPROPERTY(EditAnywhere)
	bool bRestore = true;

protected:
	UPROPERTY(EditAnywhere)
	bool bUseReplication = true;
};

UCLASS()
class TESTGAME_API UMAnimNotifyState_AddMovemntInput : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere)
	float Scale = 1.f;
	UPROPERTY(EditAnywhere)
	bool bForce = false;
};
UCLASS()
class TESTGAME_API UAnimNotify_WeaponCharge : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

class UMDamageComponent;
UCLASS()
class TESTGAME_API UMAnimNotifyState_ActivateAbilityByTag : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere)
	TArray<FName> BoneNames;

protected:
	UPROPERTY(EditAnywhere)
	float Range = 1000.f;
	UPROPERTY(EditAnywhere)
	FVector2D Angle;

	TWeakObjectPtr<AActor> Owner = nullptr;
	//TWeakObjectPtr<UMDamageComponent> DamageComponent = nullptr;
};

UCLASS()
class TESTGAME_API UAnimNotify_FootStep : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere)
	FName SocketName = NAME_None;
	UPROPERTY(EditAnywhere)
	FVector LocationOffset = FVector::ZeroVector;

};