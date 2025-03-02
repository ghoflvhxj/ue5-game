#include "MHud.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MGameState/MGameStateInGame.h"
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

	APawn* Pawn = PlayerOwner->GetPawn();
	if (IsValid(Pawn))
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
	// 모든 GE를 HUD가 처리할 필요는 없다. HUD가 알아서 필요한 것만 쓰자.
	// 예를 들어 WolfCircle 어빌리티를 실행하면 GE_Cool, GE_Duration, GE_ActualCool 이렇게 3개의 GE가 순차적으로 실행됨
	// Cue로 실행한다면 따로 구분해주지 않아도 되는데 CueParameter 관련 처리가 필요 vs 여기서 하기
	// 관전해서 뷰 타겟이 변경된다면 여기서 초기화 하는 방법밖에 없긴 함

	const FActiveGameplayEffectHandle& EffectHandle = InGameplayEffect.Handle;
	const FGameplayEffectSpec& EffectSpec = InGameplayEffect.Spec;

	FGameplayTagContainer EffectTags;
	EffectSpec.GetAllAssetTags(EffectTags);

	FGameplayTagContainer StatusableTags;
	//StatusableTags.AddTag(FGameplayTag::RequestGameplayTag("Test.GenericTag"));
	FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag("Ability");

	const FGameplayTag& Tag = EffectTags.Last();

	// 스킬 쿨
	if (Tag.MatchesTag(AbilityTag))
	{
		UpdateSkillCool(InAbilitySystemComponent, EffectSpec, EffectHandle, Tag.GetSingleTagContainer());
	}
	else
	{
		// Duration인 버프/디버프 이펙트가 오는 경우 Status를 갱신
		if (InGameplayEffect.GetDuration() != UGameplayEffect::INFINITE_DURATION && InGameplayEffect.GetDuration() != UGameplayEffect::INSTANT_APPLICATION)
		{
			TMap<FGameplayAttribute, float> MapAttributeToModified;

			for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : EffectSpec.ModifiedAttributes)
			{
				MapAttributeToModified.FindOrAdd(ModifiedAttribute.Attribute) += ModifiedAttribute.TotalMagnitude;
			}

			AddStatusEffect(InAbilitySystemComponent->GetOwner(), Tag, InGameplayEffect.IsPendingRemove, MapAttributeToModified, EffectHandle);
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

	// 라운드 바인딩
	if (AMGameStateInGame* GameStateInGame = Cast<AMGameStateInGame>(UGameplayStatics::GetGameState(this)))
	{
		GameStateInGame->GameOverDynamicDelegate.AddDynamic(this, &AMHudInGame::ShowGameOver);

		if (URoundComponent* RoundComponent = GameStateInGame->GetComponentByClass<URoundComponent>())
		{
			UpdateRoundInfo(RoundComponent->GetRoundWave());
			RoundComponent->GetWaitNextRoundEvent().AddUObject(this, &AMHudInGame::ShowGetRewardMessage);
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
		PlayerController->GetSpectateModeChangedEvent().AddUObject(this, &AMHudInGame::ShowSpectateInfo);
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
			}
		});
		
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
			PlayerState->GetPlayerDeadEvent().AddUObject(this, &AMHudInGame::ShowDieInfo);
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