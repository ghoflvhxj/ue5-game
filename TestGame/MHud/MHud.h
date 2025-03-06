#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Hud.h"
#include "AbilitySystemComponent.h"

#include "TestGame/MGameState/MGameStateInGame.h"

#include "MHud.generated.h"

class UUserWidget;
class AWeapon;
struct FRoundInfo;
struct FRound;
//struct FExperienceInfo;
struct FOnAttributeChangeData;
struct FPickData;
struct FGameplayCueParameters;
struct FSkillEnhanceData;

// 대미지 플로터 표시할 때 사용은 하고 있긴함
USTRUCT(BlueprintType)
struct FDamage
{
	GENERATED_BODY()

	float Value = 0.f;
};

UINTERFACE(BlueprintType)
class TESTGAME_API USystemWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API ISystemWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Close();
};

UCLASS()
class TESTGAME_API AMHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual bool InitializeUsingPlayerState(APlayerState* PlayerState);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> HUDWidgetClass = nullptr;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* HUDWidget = nullptr;

protected:
	void ShowWidget();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShowTimer = 0.f;
	// 설정하면 HUD BeginPlay후 UI를 보여줍니다. 해제 시 수동으로 ShowWidget을 호출해야 합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool ShowHudWidgetAfterCreation = true;

public:
	UFUNCTION(BlueprintCallable)
	void AddWidget(UUserWidget* InWidget);
	UFUNCTION(BlueprintCallable)
	void CloseWidget();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> SystemWidgetClass = nullptr;
	TArray<UUserWidget*> WidgetContainer;

};

UCLASS()
class TESTGAME_API AMHudInGame : public AMHud
{
	GENERATED_BODY()

public:
	AMHudInGame();

public:
	virtual void BeginPlay() override;
	virtual bool InitializeUsingPlayerState(APlayerState* PlayerState) override;

protected:
	UFUNCTION()
	virtual void ShowHudWidget(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION()
	virtual void UpdatePawnBoundWidget(APawn* OldPawn, APawn* NewPawn);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCharacterInfo(APawn* OldPawn, APawn* NewPawn);
	//UFUNCTION(BlueprintImplementableEvent)
	//void UpdateCharacterExperience(const FExperienceInfo& InExperienceInfo);
protected:
	FDelegateHandle HealthUpdateHandle;

	// 플로터
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowFloatingMessage(AActor* InActor, const FGameplayCueParameters& InCueParameter);
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDamageFloater(const FDamage& InDamage);

/* GAS 관련 */
public:
	// 활성화된 모든 GE로 HUD를 업데이트 함
	UFUNCTION(BlueprintCallable)
	void InitByGameplayEffect();
	// GE로 HUD를 업데이트함
	UFUNCTION()
	void UpdateByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FActiveGameplayEffect& InGameplayEffect);
	// GE로 HUD 요소를 제거함
	UFUNCTION()
	void RemoveByGameplayEffect(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InEffectSpec, FActiveGameplayEffectHandle InActiveEffectHandle);
	void Test(UAbilitySystemComponent* InAbilitySystemComponent, FGameplayTag InTag);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSkillCool(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& InEffectSpec, FActiveGameplayEffectHandle InActiveEffectHandle, const FGameplayTagContainer& SkillTags);
	// 스킬 또는 이펙트의 Duration 표시
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateEffectDuration(AActor* InTarget, int32 InSkillIndex, bool bRemove, const TMap<FGameplayAttribute, double>& InModifiedMagnitude, const FActiveGameplayEffectHandle& Handle);
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	UUserWidget* AddStatusEffect(AActor* InTarget, FGameplayTag InTag, bool bRemove, const TMap<FGameplayAttribute, float>& InModifiedMagnitude, const FActiveGameplayEffectHandle& Handle);
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveStatusEffect(FGameplayTag InTag);

	// 레벨 관련
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSkillEnhanceWidget(const FSkillEnhanceData& InData);

	// 무기 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponInfo(AActor* OldWeapon, AActor* NewWeapon);

	// 피킹
public:
	UFUNCTION(BlueprintImplementableEvent)
	void TogglePickInfo(AActor* Old, AActor* New);
	UFUNCTION(BlueprintImplementableEvent)
	void UdpatePickInfo(const FPickData& InPickData);

	// 라운드 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowGetRewardMessage();
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRoundInfo(const FRound& InRound);
	UFUNCTION(BlueprintImplementableEvent)
	void BoundBoss(AActor* Boss);

	// 게임 결과
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowGameFinish(EEndMatchReason EndMatchReason);
	UFUNCTION(BlueprintNativeEvent)
	void ShowGameOver();

	// 재화
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMoney(int32 InMoney);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateItem(int32 InIndex, int32 InNum);

	// 사망 & 관전
public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowSpectateInfo(bool InSpectating);
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDieInfo();

public:
	UFUNCTION(BlueprintPure)
	UUserWidget* GetCharactetSelectWidget() { return CharacterSelectWidget; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> CharacterSelectWidgetClass = nullptr;
	UUserWidget* CharacterSelectWidget = nullptr;
	
/* 팀원 */
public:
	UFUNCTION(BlueprintNativeEvent)
	bool AddOtherPlayer(APlayerState* InPlayerState);
	void RemoveOtherPlayer(APlayerState* InPlayerState) { OtherPlayers.Remove(InPlayerState); }
	UFUNCTION(BlueprintPure)
	const TArray<APlayerState*>& GetOtherPlayers() const { return OtherPlayers; }
protected:
	TArray<APlayerState*> OtherPlayers;
};