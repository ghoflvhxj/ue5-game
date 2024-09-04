#include "DropItem.h"

#include "TestGame/MCharacter/MCharacter.h"

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

void ADropItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (FItemBaseInfo * ItemBaseInfo = GetItemBaseInfo())
	{
		SkeletalMeshComponent->SetSkeletalMesh(ItemBaseInfo->DropMesh);
	}
}

// Called every frame
void ADropItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADropItem::Interaction_Implementation(AActor* Interactor)
{
	if (FItemBaseInfo * ItemBaseInfo = GetItemBaseInfo())
	{
		//switch (ItemBaseInfo->ItemType)
		//{
		//default:
		//break;
		//}
	}
}

void ADropItem::OnTargeted_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("상호작용 대상이 됨"));

	Interaction(Interactor);

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