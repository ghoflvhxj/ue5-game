#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Hud.h"

#include "MHud.generated.h"

class UUserWidget;
class AWeapon;
struct FRoundInfo;

UCLASS()
class TESTGAME_API AMHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual bool InitializeUsingPlayerState(APlayerState* PlayerState);

protected:
	void CreateHUDWidget();
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
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCharacterInfo(APawn* OldPawn, APawn* NewPawn);

public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowGameOver();

	// 무기 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponInfo(AWeapon* OldWeapon, AWeapon* NewWeapon);

	// 라운드 정보 갱신
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRoundInfo(const FRoundInfo& InRoundInfo);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMoney(int32 InMoney);
};