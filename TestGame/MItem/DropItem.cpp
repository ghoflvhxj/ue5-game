#include "DropItem.h"

#include "GameFramework/PlayerState.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/InteractorComponent.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MWeapon/Weapon.h"

// Sets default values
ADropItem::ADropItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetComponentTickEnabled(false);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetComponentTickEnabled(false);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(GetRootComponent());
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetComponentTickEnabled(false);

	InteractorComponent = CreateDefaultSubobject<UMInteractorComponent>(TEXT("InteractorComponent"));
	InteractorComponent->SetComponentTickEnabled(false);
}

void ADropItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ADropItem, ItemIndex))
	{
		UpdateItemMesh();
	}
}

// Called when the game starts or when spawned
void ADropItem::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractorComponent))
	{
		InteractorComponent->InteractFinishEvent.AddWeakLambda(this, [this]() {
			FItemBaseInfo* ItemBaseInfo = GetItemBaseInfo();
			AMCharacter* Interactor = InteractorComponent->GetInteractor<AMCharacter>();
			if (IsValid(Interactor) == false || ItemBaseInfo == nullptr)
			{
				return;
			}

			switch (ItemBaseInfo->ItemType)
			{
				case EItemType::Money:
				{
					if (APlayerState* PlayerState = Interactor->GetPlayerState())
					{
						if (UMInventoryComponent* InventoryComponent = PlayerState->GetComponentByClass<UMInventoryComponent>())
						{
							InventoryComponent->AddMoney(10);
						}
					}
				}
				break;
				case EItemType::Weapon:
				{
					// 장비 장착하는 거 EquipComponent로 분리하고, 그 컴포넌트를 사용하는 것으로 바꾸기
					if (AWeapon* Weapon = Cast<AWeapon>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, WeaponClass, Interactor->GetActorTransform())))
					{
						Weapon->SetWeaponIndex(ItemBaseInfo->Index);
						UGameplayStatics::FinishSpawningActor(Weapon, Interactor->GetActorTransform());
						Interactor->EquipWeapon(Weapon);

						// 클라 액터는 즉각적인 피드백을 위한 것이니 삭제되야 함
						if (HasAuthority() == false)
						{
							Weapon->SetLifeSpan(3.f);
						}
					}
				}
				break;
			}

			SetActorHiddenInGame(true);
			SetLifeSpan(0.1f);
			if (IsValid(SphereComponent))
			{
				SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SphereComponent->SetGenerateOverlapEvents(false);
			}
		});
	}

	UpdateItemMesh();
}

void ADropItem::OnRep_ItemIndex()
{
	Super::OnRep_ItemIndex();

	UpdateItemMesh();
}

void ADropItem::UpdateItemMesh()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (FItemBaseInfo* ItemBaseInfo = GetItemBaseInfo())
	{
		if (UStreamableRenderAsset* MeshAsset = ItemBaseInfo->GetMesh())
		{
			switch (MeshAsset->GetRenderAssetType())
			{
				case EStreamableRenderAssetType::SkeletalMesh:
				{
					if (IsValid(SkeletalMeshComponent) && SkeletalMeshComponent->GetSkeletalMeshAsset() != MeshAsset)
					{
						SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(MeshAsset));
					}
					if (IsValid(StaticMeshComponent))
					{
						StaticMeshComponent->SetStaticMesh(nullptr);
					}
				}
				break;
				case EStreamableRenderAssetType::StaticMesh:
				{
					if (IsValid(StaticMeshComponent) && StaticMeshComponent->GetStaticMesh() != MeshAsset)
					{
						StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(MeshAsset));
					}
					if (IsValid(SkeletalMeshComponent))
					{
						SkeletalMeshComponent->SetSkeletalMesh(nullptr);
					}
				}
				break;
			}
		}
	}
}

void ADropItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//UpdateItemMesh();
}

// Called every frame
void ADropItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//void ADropItem::Interaction_Implementation(AActor* Interactor)
//{
//	if (FItemBaseInfo * ItemBaseInfo = GetItemBaseInfo())
//	{
//		//switch (ItemBaseInfo->ItemType)
//		//{
//		//default:
//		//break;
//		//}
//	}
//}
//
//void ADropItem::OnTargeted_Implementation(AActor* Interactor)
//{
//	UE_LOG(LogTemp, Log, TEXT("상호작용 대상이 됨"));
//
//	Interaction(Interactor);
//
//	OnInteractTargetedDelegate.Broadcast(true);
//}
//
//void ADropItem::OnUnTargeted_Implementation(AActor* Interactor)
//{
//	UE_LOG(LogTemp, Log, TEXT("상호작용 대상이 해제됨"));
//	OnInteractTargetedDelegate.Broadcast(false);
//}
//
//bool ADropItem::IsInteractable_Implementation(AActor* Interactor)
//{
//	if (Interactor->IsA(AMCharacter::StaticClass()) == false)
//	{
//		return false;
//	}
//
//	return true;
//}