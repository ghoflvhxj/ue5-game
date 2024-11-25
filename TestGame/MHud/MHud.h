#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Hud.h"

#include "MHud.generated.h"

class UUserWidget;
class AWeapon;
struct FRoundInfo;
//struct FExperienceInfo;
struct FOnAttributeChangeData;

UCLASS()
class TESTGAME_API AMHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual bool InitializeUsingPlayerState(APlayerState* PlayerState);

protected:
	UFUNCTION()
	virtual void OnPawnChanged(APawn* OldPawn, APawn* NewPawn) {}

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
	virtual void OnPawnChanged(APawn* OldPawn, APawn* NewPawn) override;

public:
	void UpdateHealth(const FOnAttributeChangeData& AttributeChangeData);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateHealth"))
	void UpdateHealthProxy(float OldValue, float NewValue, float MaxValue);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCharacterInfo(APawn* OldPawn, APawn* NewPawn);
	//UFUNCTION(BlueprintImplementableEvent)
	//void UpdateCharacterExperience(const FExperienceInfo& InExperienceInfo);
protected:
	FDelegateHandle HealthUpdateHandle;

public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowGameOver();

	// 무기 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponInfo(AActor* OldWeapon, AActor* NewWeapon);

	// 라운드 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRoundInfo(int32 Round, const FRoundInfo& InRoundInfo, int32 Wave);
	UFUNCTION(BlueprintImplementableEvent)
	void BoundBoss(AActor* Boss);

	// 게임 결과
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowGameFinish();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMoney(int32 InMoney);
};