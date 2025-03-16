#include "MHud.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
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
		UpdateByGameplayEffect(AbilitySystemComponent, *Iter);
	}
}

void AMHudInGame::UpdateByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FActiveGameplayEffect& InGameplayEffect)
{
	// GE로 스킬쿨 UI를 다룬 이유가... 관전으로 시점 변경 시에도 듀레이션을 보여주고 싶었음
	// GC로 하면 가져올 방법이 딱히 없는데, 이펙트로 하면 그냥 활성화 된 이펙트들을 가져오면 됨

	const FActiveGameplayEffectHandle& ActiveEffectHandle = InGameplayEffect.Handle;
	const FGameplayEffectSpec& EffectSpec = InGameplayEffect.Spec;
	const FGameplayEffectContextHandle& EffectContextHandle = EffectSpec.GetEffectContext();

	FGameplayTagContainer EffectTags;
	EffectSpec.GetAllAssetTags(EffectTags);
	
	const FGameplayTag& Tag = EffectTags.Last();

	// 스킬 쿨
	if (EffectTags.HasAny(FGameplayTag::RequestGameplayTag("EffectType.Cool").GetSingleTagContainer()))
	{
		const FGameplayTagContainer& AbilityTags = EffectTags.Filter(FGameplayTag::RequestGameplayTag("Ability").GetSingleTagContainer());
		for (auto AbilityTag : AbilityTags)
		{
			UpdateSkillCool(InAbilitySystemComponent, EffectSpec, ActiveEffectHandle, AbilityTag.GetSingleTagContainer());
		}
	}
	
	// 듀레이션
	if (InGameplayEffect.GetDuration() != UGameplayEffect::INFINITE_DURATION && InGameplayEffect.GetDuration() != UGameplayEffect::INSTANT_APPLICATION)
	{
		if (EffectTags.HasAny(FGameplayTag::RequestGameplayTag("EffectType.Duration").GetSingleTagContainer()))
		{
			TMap<FGameplayAttribute, double> MapAttributeToModified;

			for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : EffectSpec.ModifiedAttributes)
			{
				MapAttributeToModified.FindOrAdd(ModifiedAttribute.Attribute) += ModifiedAttribute.TotalMagnitude;
			}

			int32 EffectIndex = INDEX_NONE;
			if (EffectContextHandle.Get()->GetScriptStruct() == FMGameplayEffectContext::StaticStruct())
			{
				const FMGameplayEffectContext* EffectContext = static_cast<const FMGameplayEffectContext*>(EffectContextHandle.Get());
				EffectIndex = EffectContext->EffectIndex;
			}

			if (EffectIndex > 0)
			{
				UpdateEffectDuration(InAbilitySystemComponent->GetOwner(), EffectIndex, InGameplayEffect.IsPendingRemove, MapAttributeToModified, ActiveEffectHandle);
			}
		}
	}
}

void AMHudInGame::RemoveByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InEffectSpec, FActiveGameplayEffectHandle InActiveEffectHandle)
{

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

bool AMHudInGame::AddOtherPlayer_Implementation(APlayerState* InPlayerState)
{
	if (IsValid(InPlayerState))
	{
		OtherPlayers.Add(InPlayerState);
		return true;
	}

	return false;
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
			RoundComponent->OnRoundAndWaveChangedEvent.AddUObject(this, &AMHudInGame::UpdateRoundInfo);
		}

		GameStateInGame->OnBossMonsterSet.AddUObject(this, &AMHudInGame::BoundBoss);
		GameStateInGame->OnMatchEndEvent.AddUObject(this, &AMHudInGame::ShowGameFinish);
	}

	// Player 바인딩
	if (AMPlayerControllerInGame* PlayerController = Cast<AMPlayerControllerInGame>(PlayerOwner))
	{
		APawn* PlayerCharacter = Cast<APawn>(PlayerOwner->GetViewTarget());
		ShowHudWidget(nullptr, PlayerCharacter);
		//UpdatePawnBoundWidget(nullptr, PlayerCharacter); //ShowHudWidget에서 호출하고 있음

		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::ShowHudWidget);
		//PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AMHudInGame::UpdatePawnBoundWidget); //ShowHudWidget에서 호출하고 있음

		// 관전 시 대상으로 HUD위젯 업데이트
		//PlayerController->GetSpectateModeChangedEvent().AddUObject(this, &AMHudInGame::ShowSpectateInfo);
		PlayerController->GetViewTargetChangedEvent().AddWeakLambda(this, [this, PlayerController](AActor* Old, AActor* New) {
			ShowHudWidget(nullptr, Cast<APawn>(New));

			APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();
			AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
			
			if (IsValid(PlayerState) && IsValid(GameState) && PlayerState->IsSpectator())
			{
				for (APlayerState* Temp : GameState->PlayerArray)
				{
					if (Temp->GetPawn() == New)
					{
						continue;
					}

					AddOtherPlayer(Temp);
				}
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