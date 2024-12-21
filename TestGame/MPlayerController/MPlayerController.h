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
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void OnRep_PlayerState() override;

public:
	UFUNCTION(BlueprintPure)
	float GetAngleToMouse(const FVector& InLocation);

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