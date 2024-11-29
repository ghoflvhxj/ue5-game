#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "MAnimNotify.generated.h"

class UUserWidget;

UCLASS()
class TESTGAME_API UMAnimNotify_SpawnActor : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
public:
	virtual void OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp) {}
	virtual void OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp) {}
	virtual bool GetSpawnParam(USkeletalMeshComponent* MeshComp, FActorSpawnParameters& OutSpawnTransform);
	virtual AActor* GetContextObject(USkeletalMeshComponent* MeshComp);
	virtual FTransform GetSocketTransform(USkeletalMeshComponent* MeshComp);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SpawnSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector SpawnOffset;
};

UCLASS()
class TESTGAME_API UMAnimNotify_SpawnBullet : public UMAnimNotify_SpawnActor
{
	GENERATED_BODY()

public:
	virtual void OnSpawn(AActor* InActor, USkeletalMeshComponent* MeshComp) override;
	virtual void OnSpawnFinished(AActor* InActor, USkeletalMeshComponent* MeshComp) override;
	virtual bool GetSpawnParam(USkeletalMeshComponent* MeshComp, FActorSpawnParameters& OutSpawnTransform) override;
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

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RemoveTags;
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AddTags;
	UPROPERTY(EditAnywhere)
	bool bRestore = true;
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
};