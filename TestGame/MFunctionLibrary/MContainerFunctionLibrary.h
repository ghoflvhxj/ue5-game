#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MContainerFunctionLibrary.generated.h"

UCLASS()
class TESTGAME_API UContainerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template <class K, class V>
	static void SerializeMap(TMap<K, V>& InMap, TArray<uint8>& Out)
	{
		FMemoryWriter MemoryWriter(Out, false);

		int32 Num = InMap.Num();
		MemoryWriter << Num;

		for (TPair<K, V>& Pair : InMap)
		{
			MemoryWriter << Pair.Key;
			MemoryWriter << Pair.Value;
		}
	}

	template <class K, class V>
	static void DeserializeMap(TMap<K, V>& InMap, TArray<uint8>& Out)
	{
		FMemoryReader MemoryReader(Out, false);

		int32 Num = 0;
		MemoryReader << Num;

		for (int32 i = 0; i < Num; ++i)
		{
			K Key;
			V Value;
			MemoryReader << Key;
			MemoryReader << Value;

			InMap.Emplace(Key, Value);
		}
	}

	template <class T>
	static void ShuffleArray(TArray<T>& In)
	{
		int32 NumItems = In.Num();
		for (int i = 0; i < NumItems; ++i)
		{
			Swap(In[i], In[FMath::Rand() % NumItems]);
		}
	}
};