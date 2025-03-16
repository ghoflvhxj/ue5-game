#pragma once

#include "CoreMinimal.h"
#include "MCharacter.h"
#include "TestGame/MItem/Drop.h"
#include "MMonster.generated.h"

class UPaperSprite;
class UPrimitiveComponent;
struct FActionTableRow;

USTRUCT(BlueprintType)
struct FMonsterInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath IconSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath IconTexture = nullptr;
};

USTRUCT(BlueprintType)
struct FMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftClassPath MonsterClass; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath AttributeData;

	UPROPERTY(EditDefaultsOnly)
	FSoftObjectPath ActionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSet<int32> ActionIndices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DropIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, int32> MapTagToEffectIndex;
};

USTRUCT(BlueprintType)
struct FMonsterTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMonsterInfo MonsterInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMonsterData MonsterData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer Tags;

	const static FMonsterTableRow Empty;
};

UCLASS()
class TESTGAME_API AMMonster : public AMCharacter, public IDropInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	virtual void Freeze(const FGameplayTag InTag, int32 InStack) override;
	virtual int32 GetEffectIndex(const FGameplayTag& InTag) const override;

public:
	virtual int32 GetDropIndex_Implementation() override;

public:
	const FMonsterTableRow& GetMonsterTableRow() const;
	UFUNCTION(BlueprintCallable)
	void SetMonsterIndex(int32 InIndex) { MonsterIndex = InIndex; }
	virtual int32 GetCharacterIndex() override { return MonsterIndex; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, meta = (ExposeOnSpawn = "true"))
	int32 MonsterIndex = INDEX_NONE;

public:
	virtual UPrimitiveComponent* GetWeaponCollision() override;
	virtual void SetWeaponCollisionEnable_Implementation(bool bInEnable) override;


	// 이거는 Controller로 옮기는게 맞을듯
public:
	UFUNCTION(BlueprintPure)
	const FActionTableRow& GetActionTableRow() const;
	UFUNCTION(BlueprintPure)
	int32 GetLoadedAction();
	UFUNCTION(BlueprintPure)
	TArray<int32> GetActions();
	UFUNCTION(BlueprintCallable)
	void LoadAction();
	UFUNCTION(BlueprintCallable)
	void SetAction	(int32 InActionIndex);
protected:
	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetLoadedAction)
	int32 LoadedAction = INDEX_NONE;
};

