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
	virtual void OnActorSpawned(AActor* InActor, USkeletalMeshComponent* MeshComp) {}

public:
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
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	virtual FTransform GetSocketTransform(USkeletalMeshComponent* MeshComp) override;
};