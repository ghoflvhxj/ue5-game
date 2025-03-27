#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/AssetManager.h"

#include "TestGame/MItem/ItemBase.h"

#include "MMiscFunctionLibrary.generated.h"
UCLASS()
class TESTGAME_API UMMiscFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static bool CompareUniqueNetID(APlayerState* Lhs, APlayerState* Rhs);

	UFUNCTION(BlueprintPure)
	static FString GetServerIpAddress();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* PlayUISoundByEffectIndex(const UObject* WorldContextObject, int32 InEffectIndex);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* PlayUISoundByEffectIndexWithConcurrency(const UObject* WorldContextObject, int32 InEffectIndex, USoundConcurrency* Concurrency);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void PlayBGM(const UObject* WorldContextObject, int32 InEffectIndex);

	UFUNCTION(BlueprintPure)
	static float UnwindDegree(float InDegree);

/* 아이템 관련 */
	UFUNCTION(BlueprintPure)
	static bool IsItemType(const FGameItemTableRow& Lhs, EItemType ItemType);
	
	UFUNCTION(BlueprintPure)
	static int32 GetItemMaxLevel(const FGameItemTableRow& Lhs);

/* 애셋 관련 */
	static void LoadAssetFrom(const UStruct* InStruct, const void* InSource)
	{
		if (InStruct == nullptr)
		{
			return;
		}

		if (InSource == nullptr)
		{
			return;
		}

		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		for (TFieldIterator<FProperty> Iter(InStruct); Iter; ++Iter)
		{
			const FStructProperty* StructProperty = CastField<FStructProperty>(*Iter);
			if (StructProperty == nullptr)
			{
				continue;
			}

			if (StructProperty->Struct != TBaseStructure<FSoftObjectPath>::Get())
			{
				continue;
			}

			const FSoftObjectPath* ObjectPath = StructProperty->ContainerPtrToValuePtr<FSoftObjectPath>(InSource);
			if (ObjectPath->IsValid() == false)
			{
				continue;
			}

			StreamableManager.RequestAsyncLoad(*ObjectPath, []() {});
		}
	}
};