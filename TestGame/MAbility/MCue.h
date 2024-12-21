#pragma once

#include "CoreMinimal.h"
#include "TestGame/TestGame.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayCueNotify_Actor.h"
#include "MCue.generated.h"

USTRUCT(BlueprintType)
struct TESTGAME_API FGameplayTagTextTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override
	{
		Super::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
		GameplayTag = FGameplayTag::RequestGameplayTag(InRowName);
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GameplayTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Text;
};

UCLASS()
class TESTGAME_API UGameplayCue_FloatMessage : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
};

class USphereComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;

UCLASS()
class TESTGAME_API AGameplayCue_CounterAttack : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGameplayCue_CounterAttack();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraComponent* NiagaraComponent = nullptr;

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
};