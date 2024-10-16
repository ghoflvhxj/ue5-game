#pragma once

#include "CoreMinimal.h"

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
	virtual FTransform GetSocketTransform(USkeletalMeshComponent* MeshComp) override;
};