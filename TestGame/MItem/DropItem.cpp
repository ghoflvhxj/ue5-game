#include "DropItem.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/InteractorComponent.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAbility/MAbility.h"

DECLARE_LOG_CATEGORY_CLASS(LogDropItem, Log, Log);

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

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(SphereComponent);

	InteractorComponent = CreateDefaultSubobject<UMInteractorComponent>(TEXT("InteractorComponent"));
	InteractorComponent->SetComponentTickEnabled(false);
}

#if WITH_EDITOR
void ADropItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ADropItem, ItemIndex))
	{
		UpdateItemMesh();
	}
}
#endif

// Called when the game starts or when spawned
void ADropItem::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractorComponent))
	{
		InteractorComponent->InteractFinishEvent.AddWeakLambda(this, [this]() {
			FGameItemTableRow* ItemBaseInfo = GetItemTableRow();
			AMCharacter* Interactor = InteractorComponent->GetInteractor<AMCharacter>();
			if (IsValid(Interactor) == false || ItemBaseInfo == nullptr)
			{
				return;
			}

			switch (ItemBaseInfo->GameItemInfo.ItemType)
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
					if (AWeapon* Weapon = Cast<AWeapon>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, WeaponClass, Interactor->GetActorTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, Interactor)))
					{
						Weapon->SetWeaponIndex(ItemBaseInfo->Index);
						UGameplayStatics::FinishSpawningActor(Weapon, Interactor->GetActorTransform());
						Weapon->SetEquipActor(Interactor);

						if (IsNetMode(NM_Client))
						{
							SetLifeSpan(0.3f);
						}
					}
				}
				break;
				case EItemType::Common:
				{
					UAbilitySystemComponent* AbilitySystemComponent = Interactor->GetComponentByClass<UAbilitySystemComponent>();
					if (HasAuthority() && IsValid(AbilitySystemComponent))
					{
						for (const auto& Pair : ItemBaseInfo->GameItemData.GameplayEffects)
						{
							FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Pair.Key, 0.f, AbilitySystemComponent->MakeEffectContext());
							for (const auto& TagToValuePair : Pair.Value.ParamTagToValueMap)
							{
								EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, TagToValuePair.Key, TagToValuePair.Value);
							}

							if (EffectSpecHandle.IsValid() == false)
							{
								UE_LOG(LogDropItem, Warning, TEXT("Faield to apply item(%d) effect."), ItemIndex);
								continue;
							}

							AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
						}

						UE_LOG(LogTemp, Warning, TEXT("GiveAbilitiy"));
						if (UMAbilityDataAsset* AbilitySet = ItemBaseInfo->GameItemData.AbilitySet)
						{
							TMap<FGameplayTag, FGameplayAbilitySpecHandle> TagToAbilityMap;
							AbilitySet->GiveAbilities(AbilitySystemComponent, TagToAbilityMap);
						}

						for (const auto& AbilityClass : ItemBaseInfo->GameItemData.Abilities)
						{
							FGameplayAbilitySpec AbilitySpec(AbilityClass);
							AbilitySystemComponent->GiveAbility(AbilitySpec);
						}
					}
				}
				break;
			}

			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			SetLifeSpan(0.1f);
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

	if (IsValid(SkeletalMeshComponent))
	{
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		NiagaraComponent->SetHiddenInGame(true, false);
	}
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		NiagaraComponent->SetHiddenInGame(true, false);
	}
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetAsset(nullptr);
		NiagaraComponent->SetHiddenInGame(true, false);
	}

	UPrimitiveComponent* ActualPrimitiveComponent = nullptr;

	if (FGameItemTableRow* ItemBaseInfo = GetItemTableRow())
	{
		if (UStreamableRenderAsset* RenderAsset = ItemBaseInfo->GetWorldAsset<UStreamableRenderAsset>())
		{
			switch (RenderAsset->GetRenderAssetType())
			{
				case EStreamableRenderAssetType::SkeletalMesh:
				{
					if (IsValid(SkeletalMeshComponent) && SkeletalMeshComponent->GetSkeletalMeshAsset() != RenderAsset)
					{
						SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(RenderAsset));
						ActualPrimitiveComponent = SkeletalMeshComponent;
					}
				}
				break;
				case EStreamableRenderAssetType::StaticMesh:
				{
					if (IsValid(StaticMeshComponent) && StaticMeshComponent->GetStaticMesh() != RenderAsset)
					{
						StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(RenderAsset));
						ActualPrimitiveComponent = StaticMeshComponent;
					}
				}
				break;
				case EStreamableRenderAssetType::Texture:
				{
					if (IsValid(StaticMeshComponent))
					{
						StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(DefaultMeshPath.TryLoad()));
						if (UMaterialInstanceDynamic* MaterialInstanceDynamic = StaticMeshComponent->CreateDynamicMaterialInstance(0))
						{
							MaterialInstanceDynamic->SetTextureParameterValue("BaseColor", Cast<UTexture>(RenderAsset));
						}
						ActualPrimitiveComponent = StaticMeshComponent;
					}
				}
				break;
			}
		}
		else if (UNiagaraSystem* NiagaraAsset = ItemBaseInfo->GetWorldAsset<UNiagaraSystem>())
		{
			if (IsValid(NiagaraComponent))
			{
				NiagaraComponent->SetAsset(NiagaraAsset);
				ActualPrimitiveComponent = NiagaraComponent;
			}
		}
	}

	if (IsValid(ActualPrimitiveComponent))
	{
		ActualPrimitiveComponent->SetHiddenInGame(false, true);
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