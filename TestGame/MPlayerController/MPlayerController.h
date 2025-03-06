#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPlayerController.generated.h"

UCLASS()
class TESTGAME_API AMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

public:
	UFUNCTION()
	void CloseUIOrOpenSystem();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputAction = nullptr;
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

struct FSkillEnhanceData;

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

	// 폰->마우스로의 Yaw
public:
	UFUNCTION(Server, Unreliable)
	void Server_SetYawToMouse(float InYaw);
	UFUNCTION(BlueprintPure)
	float YawToMouseFromWorldLocation(const FVector& InLocation);
	UFUNCTION(BlueprintPure)
	float GetYawToMouse() { return YawToMouseFromPawn; }
	UFUNCTION(BlueprintPure)
	FVector GetDirectionToMouse();
	FVector DirectionToMouseFromPawn = FVector::ZeroVector;
	float YawToMouseFromPawn = 0.f;
	float LastSendYawToMouseFromPawn = 0.f;
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawYaw = false;
#endif

	// 배경 오브젝트 투명화
protected:
	TSet<TWeakObjectPtr<UPrimitiveComponent>> MaskedPrimitives;

	// 캐릭터 인덱스 관련
public:
	UFUNCTION(BlueprintCallable)
	void SetCharacterIndex(int32 InIndex);
	UFUNCTION(Server, Reliable)
	void Server_CharacterSelect(int32 InIndex);
	UFUNCTION(BlueprintPure)
	int32 GetCharacterIndex() const;

	// 준비 관련
public:
	bool IsReady() const { return bReady; }
	UFUNCTION(Server, Reliable)
	void Server_Ready();
protected:
	mutable bool bReady = false;

	// 레벨업 관련
public:
	UFUNCTION(Client, Reliable)
	void Client_SkillEnhance(const FSkillEnhanceData& InEnhanceData);

public:
	void UpdatePickInfo();
};

UINTERFACE(BlueprintType)
class TESTGAME_API UPickInterface : public UInterface
{
	GENERATED_BODY()
};

class TESTGAME_API IPickInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	FVector GetLocation();
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
	bool bPickInterface = false;
	FOnPickingChangedEvent OnPickChangedEvent;
	FOnPickDataChangedEvent OnPickDataChangedEvent;
};