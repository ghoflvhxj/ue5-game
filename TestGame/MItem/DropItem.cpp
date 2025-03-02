#include "DropItem.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

#include "CharacterLevelSubSystem.h"
#include "TestGame/MGameState/MGameStateInGame.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MCharacter/Component/InteractorComponent.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MWeapon/Weapon.h"
#include "TestGame/MAbility/MAbility.h"
#include "TestGame/MAbility/MItemAbility.h"
#include "SkillSubsystem.h"

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
		InteractorComponent->InteractStartEvent.AddWeakLambda(this, [this]() {
			FGameItemTableRow* ItemBaseInfo = GetItemTableRow();
			if (ItemBaseInfo == nullptr)
			{
				return;
			}

			if (ItemBaseInfo->GameItemInfo.ItemType == EItemType::Exp)
			{
				bGoToInteractor = true;
				if (IsValid(ActualPrimitiveComponent))
				{
					ActualPrimitiveComponent->SetSimulatePhysics(false);
				}
				SetActorTickEnabled(true);
			}
		});

		InteractorComponent->InteractFinishEvent.AddWeakLambda(this, [this](bool bSuccess) {
			FGameItemTableRow* ItemBaseInfo = GetItemTableRow();
			AMCharacter* Interactor = InteractorComponent->GetInteractor<AMCharacter>();
			if (IsValid(Interactor) == false || ItemBaseInfo == nullptr)
			{
				return;
			}

			if (bSuccess == false)
			{
				return;
			}

			if (IsNetMode(NM_DedicatedServer) == false)
			{
				if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
				{
					if (ItemBaseInfo->GameItemInfo.AcquireSound.IsValid() && PlayerController->GetViewTarget() == Interactor)
					{
						UGameplayStatics::PlaySound2D(this, Cast<USoundBase>(ItemBaseInfo->GameItemInfo.AcquireSound.TryLoad()));
					}
				}
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
				case EItemType::Exp:
				{
					GetWorld()->GetSubsystem<UCharacterLevelSubSystem>()->AddExperiance(Interactor, 1); 
				}
				break;
				case EItemType::Weapon:
				{
					// 장비 장착하는 거 EquipComponent로 분리하고, 그 컴포넌트를 사용하는 것으로 바꾸기
					if (AWeapon* Weapon = Cast<AWeapon>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, WeaponClass, Interactor->GetActorTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, Interactor)))
					{
						Weapon->SetInstigator(Interactor);
						Weapon->SetWeaponIndex(ItemBaseInfo->Index);
						UGameplayStatics::FinishSpawningActor(Weapon, Interactor->GetActorTransform());
						Weapon->SetEquipActor(Interactor);

						if (IsNetMode(NM_Client)) // 아이템 획득은 서버에서만 처리하고 있어서 의미가 없음
						{
							SetLifeSpan(0.3f);
						}
					}
				}
				break;
				case EItemType::Common:
				case EItemType::Item:
				{
					if (UAbilitySystemComponent* AbilitySystemComponent = Interactor->GetComponentByClass<UAbilitySystemComponent>())
					{
						// 아이템 버프 적용
						for (const auto& Pair : ItemBaseInfo->GameItemData.GameplayEffects)
						{
							FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Pair.Key, 0.f, AbilitySystemComponent->MakeEffectContext());
							const FGameplayEffectParam& EffectParam = Pair.Value;
							for (const auto& TagToValuePair : EffectParam.MapParamToValue)
							{
								EffectSpecHandle = UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, TagToValuePair.Key, TagToValuePair.Value);
							}

							const TArray<UAbilitySystemComponent*>& Targets = GetItemEffectTargets(EffectParam);

							if (EffectSpecHandle.IsValid() == false)
							{
								UE_LOG(LogDropItem, Warning, TEXT("Faield to apply item(%d) effect."), ItemIndex);
								continue;
							}

							if (UGameplayEffect* GameplayEffect = Pair.Key->GetDefaultObject<UGameplayEffect>())
							{
								if (GameplayEffect->GameplayCues.Num() > 0)
								{
									const TArray<FGameplayTag>& GameplayCueTags = GameplayEffect->GameplayCues[0].GameplayCueTags.GetGameplayTagArray();
									for (const FGameplayTag& GameplayCueTag : GameplayCueTags)
									{
										EffectSpecHandle.Data.Get()->AddDynamicAssetTag(GameplayCueTag);
									}
								}
							}

							for (UAbilitySystemComponent* Target : Targets)
							{
								AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), Target);
							}
						}

						// 아이템 능력 적용
						if (UMAbilityDataAsset* AbilitySet = ItemBaseInfo->GameItemData.AbilitySet)
						{
							Interactor->AddAbilities(AbilitySet);

							for (const FMAbilityBindInfo& AbilityBind : AbilitySet->Abilities)
							{
								FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityBind.GameplayAbilityClass);
								if (AbilitySpec == nullptr)
								{
									continue;
								}

								UGameplayAbility_Item* ItemAbility = Cast<UGameplayAbility_Item>(AbilitySpec->GetPrimaryInstance());
								if (IsValid(ItemAbility) == false)
								{
									continue;
								}

								int32 ItemAbilityLevel = ItemAbility->GetAbilityLevel();
								ItemAbility->SetParams(ItemBaseInfo->GameItemData.GetParam(ItemAbilityLevel));
							}
						}
					}
					
					if (ItemBaseInfo->GameItemInfo.ItemType == EItemType::Item)
					{
						Interactor->AddItem(ItemIndex, 1);
					}
					else if (ItemBaseInfo->GameItemInfo.ItemType == EItemType::Common)
					{
						Interactor->UseItem(ItemIndex);
					}
				}
				break;
			}

			if (bGoToInteractor)
			{
				// 서버에서 팔로우가 끝나서 클라에서 따라가는 걸 보여주려면 클라에서만 Visibility 업데이트
				if (IsNetMode(NM_Standalone) || HasAuthority() == false)
				{
					SetActorHiddenInGame(true);
				}
			}
			else
			{
				SetActorHiddenInGame(true);
			}

			SetLifeSpan(3.f);
			SetActorEnableCollision(false);
		});
	}

	UpdateItemMesh();
}

TArray<UAbilitySystemComponent*> ADropItem::GetItemEffectTargets(const FGameplayEffectParam& InEffectParam)
{
	TArray<UAbilitySystemComponent*> Out;

	switch (InEffectParam.Target)
	{
		case EIGameplayEffectTarget::Player:
		{
			AMCharacter* Interactor = InteractorComponent->GetInteractor<AMCharacter>();
			if (IsValid(Interactor) == false)
			{
				break;
			}

			if (UAbilitySystemComponent* AbilitySystemComponent = Interactor->GetAbilitySystemComponent())
			{
				Out.Add(AbilitySystemComponent);
			}
		}
		break;
		case EIGameplayEffectTarget::AllPlayer:	// AllPlayer말고 범위에 기반한 플레이어는 어떤지?
		{
			AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
			if (IsValid(GameState) == false)
			{
				break;
			}

			for (APlayerState* PlayerState : GameState->PlayerArray)
			{
				AMCharacter* Character = PlayerState->GetPawn<AMCharacter>();
				if (IsValid(Character) == false)
				{
					continue;
				}

				if (UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent())
				{
					Out.Add(AbilitySystemComponent);
				}
			}
		}
		break;
		case EIGameplayEffectTarget::AllMonster:
		{
			AMGameStateInGame* GameState = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this));
			if (IsValid(GameState) == false)
			{
				break;
			}

			const TSet<AActor*> Monsters = GameState->GetMonsters();
			Out.Reserve(Monsters.Num());
			for (AActor* Monster : Monsters)
			{
				if (AMCharacter* Character = Cast<AMCharacter>(Monster))
				{
					if (UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent())
					{
						Out.Add(AbilitySystemComponent);
					}
				}
			}
		}
		break;
	}

	return Out;
}

void ADropItem::OnRep_ItemIndex()
{
	Super::OnRep_ItemIndex();

	UpdateItemMesh();
}

void ADropItem::UpdateItemMesh_Implementation()
{
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
}

void ADropItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGoToInteractor && InteractorComponent->GetInteractor<AActor>())
	{
		FVector ToInteractor = InteractorComponent->GetInteractor<AActor>()->GetActorLocation() - ActualPrimitiveComponent->GetComponentLocation();
		FVector DirectionToInteractor = ToInteractor.GetSafeNormal();
		
		float MoveSpeed = 500.f * FMath::Clamp(FollowingElapsedTime + 1.f, 1.f, 5.f);
		if (MoveSpeed * DeltaTime > ToInteractor.Length())
		{
			// MoveSpeed = ToInteractor.Length() / DeltaTime; 굳이? 그냥 끝내버리자
			InteractorComponent->SuccessInteract();
			SetActorTickEnabled(false);
		}

		FollowingElapsedTime += 2.f * DeltaTime;

		ActualPrimitiveComponent->SetWorldLocation(ActualPrimitiveComponent->GetComponentLocation() + DirectionToInteractor * MoveSpeed * DeltaTime);
	}
}