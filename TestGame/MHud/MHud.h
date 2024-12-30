#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Hud.h"

#include "MHud.generated.h"

class UUserWidget;
class AWeapon;
struct FRoundInfo;
struct FRound;
//struct FExperienceInfo;
struct FOnAttributeChangeData;
struct FPickData;
struct FGameplayCueParameters;

USTRUCT(BlueprintType)
struct FDamage
{
	GENERATED_BODY()

	float Value = 0.f;
};

UCLASS()
class TESTGAME_API AMHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual bool InitializeUsingPlayerState(APlayerState* PlayerState);

protected:
	UFUNCTION()
	virtual void UpdatePawnBoundWidget(APawn* OldPawn, APawn* NewPawn) {}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* HUDWidget;

protected:
	void ShowWidget();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShowTimer;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool ShowHudWidgetAfterCreation = true;
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
	virtual void UpdatePawnBoundWidget(APawn* OldPawn, APawn* NewPawn) override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCharacterInfo(APawn* OldPawn, APawn* NewPawn);
	//UFUNCTION(BlueprintImplementableEvent)
	//void UpdateCharacterExperience(const FExperienceInfo& InExperienceInfo);
protected:
	FDelegateHandle HealthUpdateHandle;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowFloatingMessage(AActor* InActor, const FGameplayCueParameters& InCueParameter);
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDamageFloater(const FDamage& InDamage);

	// 무기 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponInfo(AActor* OldWeapon, AActor* NewWeapon);

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
	void ShowGameFinish();
	UFUNCTION(BlueprintNativeEvent)
	void ShowGameOver();

	// 재화
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMoney(int32 InMoney);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSpectateInfo(bool InSpectating);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDieInfo();
};