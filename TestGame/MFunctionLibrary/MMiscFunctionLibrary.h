#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/AssetManager.h"
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
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(*Iter))
			{
				if (StructProperty->Struct == TBaseStructure<FSoftObjectPath>::Get())
				{
					if (const FSoftObjectPath* ObjectPath = StructProperty->ContainerPtrToValuePtr<FSoftObjectPath>(InSource))
					{
						if (ObjectPath->IsValid())
						{
							StreamableManager.RequestAsyncLoad(*ObjectPath, []() {});
						}
					}
				}
			}
		}
	}
};