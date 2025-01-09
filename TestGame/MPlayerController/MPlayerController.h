#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPlayerController.generated.h"

UCLASS()
class TESTGAME_API AMPlayerController : public APlayerController
{
	GENERATED_BODY()
};

UCLASS()
class TESTGAME_API AMPlayerControllerTitle : public AMPlayerController
{
	GENERATED_BODY()

public:
	//virtual void BeginPlay() override;
};

DECLARE_EVENT_OneParam(AMPlayerController, FOnSpectateModeChangedEvent, bool)
DECLARE_EVENT_TwoParams(AMPlayerControllerInGame, FOnViewTargetChangedEvent, AActor*, AActor*);

UCLASS()
class TESTGAME_API AMPlayerControllerInGame : public AMPlayerController
{
	GENERATED_BODY()

public:
	AMPlayerControllerInGame();
protected:
	UPROPERTY(BlueprintReadOnly)
	UPickComponent* PickComponent = nullptr;

public:
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	virtual void OnRep_PlayerState() override;
protected:
	virtual void BeginPlay() override;

protected:
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
	virtual bool ShouldKeepCurrentPawnUponSpectating() const override { return true; }
	virtual void BeginSpectatingState() override;
	virtual void EndSpectatingState() override;

public:
	UFUNCTION(BlueprintCallable)
	void StartSpectate();

public:
	FOnViewTargetChangedEvent& GetViewTargetChangedEvent() { return OnViewTargetChangedEvent; }
	FOnSpectateModeChangedEvent& GetSpectateModeChangedEvent() { return OnSpectateModeChangedEvent; }
protected:
	FOnViewTargetChangedEvent OnViewTargetChangedEvent;
	FOnSpectateModeChangedEvent OnSpectateModeChangedEvent;

public:
	UFUNCTION(Server, Unreliable)
	void Server_SetYawToMouse(float InYaw);
	UFUNCTION(BlueprintPure)
	float YawToMouseFromWorldLocation(const FVector& InLocation);
	UFUNCTION(BlueprintPure)
	float GetYawToMouse() { return YawToMouseFromPawn; }
	float YawToMouseFromPawn = 0.f;
	float LastSendYawToMouseFromPawn = 0.f;

public:
	bool IsReady() const { return bReady; }
	UFUNCTION(Server, Reliable)
	void Server_Ready();
protected:
	mutable bool bReady = false;
};

USTRUCT(BlueprintType)
struct FPickData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ScreenLocation = FVector2D::ZeroVector;
};

DECLARE_EVENT_TwoParams(UMousePickableComponent, FOnPickingChangedEvent, AActor* Old, AActor* New);
DECLARE_EVENT_OneParam(UMousePickableComponent, FOnPickDataChangedEvent, const FPickData& PickData);

// 마우스를 이용해 액터를 피킹할 때 사용되는 컴포넌트
UCLASS()
class TESTGAME_API UPickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPickComponent();

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool IsPicking(AActor* InActor);
	bool IsPickable(AActor* InActor);
	void SetPickingActor(AActor* InActor);
	FOnPickingChangedEvent& GetPickChangedEvent() { return OnPickChangedEvent; }
	FOnPickDataChangedEvent& GetPickDataChangedEvent() { return OnPickDataChangedEvent; }
protected:
	mutable TWeakObjectPtr<AActor> PickingActor = nullptr;
	FOnPickingChangedEvent OnPickChangedEvent;
	FOnPickDataChangedEvent OnPickDataChangedEvent;
};