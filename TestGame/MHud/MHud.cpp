#include "MHud.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "TestGame/MAbility/MAbilitySystemComponent.h"
#include "TestGame/MCharacter/MCharacter.h"
#include "TestGame/MComponents/InventoryComponent.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "TestGame/MSpawner/MonsterSpawner.h"
#include "TestGame/MPlayerController/MPlayerController.h"
#include "MyPlayerState.h"
#include "AttributeDisplayWidget.h" // ActorBindWidget 참조
#include "Blueprint/WidgetBlueprintLibrary.h"
//#include "CharacterLevelSubSystem.h"

void AMHud::BeginPlay()
{
	Super::BeginPlay();

	if (ShowHudWidgetAfterCreation)
	{
		ShowWidget();
	}
}

bool AMHud::InitializeUsingPlayerState(APlayerState* PlayerState)
{
	return IsValid(PlayerState);
}

void AMHud::ShowWidget()
{
	if (IsValid(HUDWidget))
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	if (IsValid(HUDWidget) == false && IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget(GetWorld(), HUDWidgetClass);
	}

	if (IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}

void AMHud::AddWidget(UUserWidget* InWidget)
{
	if (IsValid(InWidget) == false)
	{
		return;
	}

	WidgetContainer.Add(InWidget);

	InWidget->AddToViewport();
}

void AMHud::CloseWidget()
{
	if (WidgetContainer.Num() > 0)
	{
		if (UUserWidget* Widget = WidgetContainer.Pop())
		{
			if (Widget->GetClass()->ImplementsInterface(USystemWidgetInterface::StaticClass()))
			{
				ISystemWidgetInterface::Execute_Close(Widget);
			}
			else
			{
				Widget->RemoveFromParent();
			}
		}
	}
	else if(IsValid(SystemWidgetClass))
	{
		if (UUserWidget* Widget = CreateWidget(GetWorld(), SystemWidgetClass))
		{
			WidgetContainer.Add(Widget);
			Widget->AddToViewport();
		}
	}

	if (APawn* Pawn = PlayerOwner->GetPawn())
	{
		if (WidgetContainer.Num() > 0)
		{
			PlayerOwner->GetPawn()->DisableInput(PlayerOwner);
		}
		else
		{
			PlayerOwner->GetPawn()->EnableInput(PlayerOwner);
		}
	}
}

void AMHud::CloseAllWidget()
{
	while (WidgetContainer.Num() > 0)
	{
		CloseWidget();
	}
}

void AMHudInGame::InitByGameplayEffect()
{
	if (IsValid(PlayerOwner) == false)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(PlayerOwner->GetViewTarget());
	if (IsValid(Pawn) == false)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilitySystemComponent) == false)
	{
		return;
	}

	const FActiveGameplayEffectsContainer& ActiveEffectsContainer = AbilitySystemComponent->GetActiveGameplayEffects();
	for (auto Iter = ActiveEffectsContainer.CreateConstIterator(); Iter; ++Iter)
	{
		const FActiveGameplayEffect& ActiveGameplayEffect = *Iter;
		UpdateByGameplayEffect(AbilitySystemComponent, ActiveGameplayEffect.Spec, ActiveGameplayEffect.Handle);
	}
}

void AMHudInGame::UpdateByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InGameplayEffectSpec, FActiveGameplayEffectHandle InActiveGameplayEffectHandle)
{
	FGameplayTagContainer EffectTags;
	InGameplayEffectSpec.GetAllAssetTags(EffectTags);

	// 스킬 쿨
	if (EffectTags.HasTag(FGameplayTag::RequestGameplayTag("EffectType.Cool")))
	{
		const FGameplayTagContainer& AbilityTags = EffectTags.Filter(FGameplayTag::RequestGameplayTag("Ability").GetSingleTagContainer());
		for (auto AbilityTag : AbilityTags)
		{
			UpdateSkillCool(InAbilitySystemComponent, InGameplayEffectSpec, InActiveGameplayEffectHandle, AbilityTag.GetSingleTagContainer());
		}
	}
	
	// 듀레이션
	float Duration = UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectTotalDuration(InActiveGameplayEffectHandle);
	if (Duration != UGameplayEffect::INFINITE_DURATION && Duration != UGameplayEffect::INSTANT_APPLICATION && EffectTags.HasTag(FGameplayTag::RequestGameplayTag("EffectType.Duration")))
	{
		const FGameplayEffectContextHandle& EffectContextHandle = InGameplayEffectSpec.GetEffectContext();

		int32 EffectIndex = INDEX_NONE;
		if (EffectContextHandle.Get()->GetScriptStruct() == FMGameplayEffectContext::StaticStruct())
		{
			const FMGameplayEffectContext* EffectContext = static_cast<const FMGameplayEffectContext*>(EffectContextHandle.Get());
			EffectIndex = EffectContext->EffectIndex;
		}

		if (EffectIndex != INDEX_NONE)
		{
			TMap<FGameplayAttribute, double> MapAttributeToModified;
			for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : InGameplayEffectSpec.ModifiedAttributes)
			{
				MapAttributeToModified.FindOrAdd(ModifiedAttribute.Attribute) += ModifiedAttribute.TotalMagnitude;
			}

			if (const FActiveGameplayEffect* ActiveEffect = InAbilitySystemComponent->GetActiveGameplayEffect(InActiveGameplayEffectHandle))
			{
				UpdateEffectDuration(InAbilitySystemComponent->GetOwner(), EffectIndex, MapAttributeToModified, InActiveGameplayEffectHandle);
			}
		}
	}
}

void AMHudInGame::RemoveByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FActiveGameplayEffect& InRemovedActiveEffect)
{
	if (IsValid(InAbilitySystemComponent) == false)
	{
		return;
	}

	RemoveEffectDuration(InAbilitySystemComponent->GetOwner(), InRemovedActiveEffect.Handle);
}

void AMHudInGame::Test(UAbilitySystemComponent* InAbilitySystemComponent, FGameplayTag InTag)
{
	//TArray<FGameplayEffectSpec> ActiveEffetSpecs;
	//InAbilitySystemComponent->GetAllActiveGameplayEffectSpecs(ActiveEffetSpecs);
	//const TArray<FActiveGameplayEffectHandle>& ActiveEffectHandles = InAbilitySystemComponent->GetActiveGameplayEffects().GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAllEffectTags(InTag.GetSingleTagContainer()));

	//FActiveGameplayEffectHandle ActiveEffectHandle;
	//for (const FActiveGameplayEffectHandle& TempActiveEffectHandle : ActiveEffectHandles)
	//{
	//	if (TempActiveEffectHandle.IsValid())
	//	{
	//		ActiveEffectHandle = TempActiveEffectHandle;
	//		break;
	//	}
	//}

	//if (ActiveEffectHandle == FActiveGameplayEffectHandle(0))
	//{
	//	return;
	//}

	const TArray<FActiveGameplayEffectHandle>& ActiveEffectHandles = InAbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAllEffectTags(InTag.GetSingleTagContainer()));
	if (ActiveEffectHandles.IsEmpty())
	{
		return;
	}

	const FActiveGameplayEffectHandle& ActiveEffectHandle = ActiveEffectHandles[0];
	if (const FActiveGameplayEffect* ActiveGameplayEffect = InAbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle))
	{
		UpdateSkillCool(InAbilitySystemComponent, ActiveGameplayEffect->Spec, ActiveGameplayEffect->Handle, InTag.GetSingleTagContainer());
	}
}

void AMHudInGame::ShowGameOver_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!!!"));
}

void AMHudInGame::ShowSpectateInfo_Implementation(bool InSpectating)
{
	AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(PlayerOwner);
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	if (InSpectating)
	{
		//UpdatePawnBoundWidget(nullptr, Cast<APawn>(PlayerController->GetViewTarget()));
	}
}

bool AMHudInGame::AddPlayer_Implementation(APlayerState* InPlayerState, AMCharacter* InCharacter)
{
	Players.Add(InPlayerState);

	if (UAbilitySystemComponent* AbilitySystemComponent = InCharacter->GetAbilitySystemComponent())
	{
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &AMHudInGame::UpdateByGameplayEffect);
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddWeakLambda(this, [this, AbilitySystemComponent](const FActiveGameplayEffect& InRemovedActiveEffect) {
			RemoveByGameplayEffect(AbilitySystemComponent, InRemovedActiveEffect);
		});
	}

	return true;
}

AMHudInGame::AMHudInGame()
{
	ShowHudWidgetAfterCreation = false;
}

void AMHudInGame::BeginPlay()
{
	Super::BeginPlay();

	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		// 라운드 바인딩
		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			UpdateRoundInfo(RoundComponent->GetRoundWave());
			RoundComponent->GetWaitNextRoundEvent().AddUObject(this, &AMHudInGame::ShowGetRewardMessage);
			RoundComponent->GetRoundPausedEvent().AddUObject(this, &AMHudInGame::UpdateRoundPause);
			RoundComponent->GetRoundAndWaveChangedEvent().AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}

		GameStateInGame->OnBossMonsterSet.AddUObject(this, &AMHudInGame::BoundBoss);
		GameStateInGame->OnMatchEndEvent.AddUObject(this, &AMHudInGame::ShowGameFinish);
	}

	// Player 바인딩
	if (AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(PlayerOwner))
	{
		APawn* PlayerCharacter = Cast<APawn>(PlayerOwner->GetViewTarget());
		ShowHudWidget(nullptr, PlayerCharacter);

		//PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::ShowHudWidget);

		// 관전 시 대상으로 HUD위젯 업데이트
		PlayerController->GetViewTargetChangedEvent().AddWeakLambda(this, [this, PlayerController](AActor* Old, AActor* New) {
			ShowHudWidget(nullptr, Cast<APawn>(New));
			AMPlayerState* PlayerState = PlayerController->GetPlayerState<AMPlayerState>();
			AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
			
			if (IsValid(GameState))
			{
				for (APlayerState* TempPlayerState : GameState->PlayerArray)
				{
					if (TempPlayerState->GetPawn() == New)
					{
						continue;
					}

					if (AMPlayerState* OtherPlayerState = Cast<AMPlayerState>(TempPlayerState))
					{
						OtherPlayerState->AddToHUD();
					}
				}
			}
			if (IsValid(PlayerState) && PlayerState->IsSpectator())
			{
				ShowSpectateInfo(true);
			}
		});
		
		if (UPickComponent* PickComponent = PlayerController->GetComponentByClass<UPickComponent>())
		{
			PickComponent->GetPickChangedEvent().AddUObject(this, &AMHudInGame::TogglePickInfo);
			PickComponent->GetPickDataChangedEvent().AddUObject(this, &AMHudInGame::UdpatePickInfo);
		}
	}
}

bool AMHudInGame::InitializeUsingPlayerState(APlayerState* InPlayerState)
{
	if (Super::InitializeUsingPlayerState(InPlayerState))
	{
		if (UMInventoryComponent* InventoryComponent = InPlayerState->GetComponentByClass<UMInventoryComponent>())
		{
			InventoryComponent->OnMoneyChangedEvent.AddUObject(this, &AMHudInGame::UpdateMoney);
		}

		if (AMPlayerState* PlayerState = Cast<AMPlayerState>(InPlayerState))
		{
			if (PlayerOwner->GetPlayerState<APlayerState>() == InPlayerState)
			{
				PlayerState->GetPlayerDeadEvent().AddUObject(this, &AMHudInGame::ShowDieInfo);
			}
		}

		return true;
	}

	return false;
}

void AMHudInGame::ShowHudWidget(APawn* OldPawn, APawn* NewPawn)
{
	if (UWorld* World = GetWorld())
	{
		if (World->bIsTearingDown)
		{
			return;
		}
	}

	if (bool bPlayerCharacterSpawned = IsValid(OldPawn) == false && IsValid(NewPawn))
	{
		ShowWidget();

		// 캐릭터 선택 위젯은 삭제
		if (IsValid(CharacterSelectWidget))
		{
			CharacterSelectWidget->RemoveFromParent();
			CharacterSelectWidget = nullptr;
		}

		UpdatePawnBoundWidget(OldPawn, NewPawn);
	}

	if (IsValid(OldPawn) == false && IsValid(NewPawn) == false)
	{
		if (IsValid(CharacterSelectWidgetClass) && IsValid(CharacterSelectWidget) == false)
		{
			CharacterSelectWidget = CreateWidget(GetWorld(), CharacterSelectWidgetClass);
		}

		if (IsValid(CharacterSelectWidget))
		{
			CharacterSelectWidget->AddToViewport();
		}
	}
}

void AMHudInGame::UpdatePawnBoundWidget(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn) == false)
	{
		return;
	}

	if (AMCharacter* Character = Cast<AMCharacter>(NewPawn))
	{
		if (UMInventoryComponent* InventoryComponent = Character->GetInventoryComponent())
		{
			InventoryComponent->GetItemAddedEvent().AddUObject(this, &AMHudInGame::UpdateItem);
		}
	}

	if (IsValid(HUDWidget))
	{
		if (UWidgetTree* WidgetTree = HUDWidget->WidgetTree)
		{
			WidgetTree->ForWidgetAndChildren(HUDWidget->GetRootWidget(), [this, NewPawn](UWidget* Widget) {
				TArray<UWidget*> UpdatedWidget;

				if (UpdatedWidget.Contains(Widget->GetParent()))
				{
					return;
				}

				if (UActorBindWidget* ActorBindWidget = Cast<UActorBindWidget>(Widget))
				{
					if (ActorBindWidget->IsAutoUpdate())
					{
						ActorBindWidget->BindActor(NewPawn);
						UpdatedWidget.Add(ActorBindWidget);
					}
				}
			});
		}
	}

	// 내가 보는 대상은 업데이트 되겠지만 다른 팀원들은?
	InitByGameplayEffect();

}