#include "DropItem.h"

#include "Client/MCharacter/MCharacter.h"

// Sets default values
ADropItem::ADropItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ADropItem::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADropItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADropItem::OnTargeted_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("상호작용 대상이 됨"));
	OnInteractTargetedDelegate.Broadcast(true);
}

void ADropItem::OnUnTargeted_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("상호작용 대상이 해제됨"));
	OnInteractTargetedDelegate.Broadcast(false);
}

bool ADropItem::IsInteractable_Implementation(AActor* Interactor)
{
	if (Interactor->IsA(AMCharacter::StaticClass()) == false)
	{
		return false;
	}

	return true;
}