// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "UObject/Script.h"
#include "CommonHeader.generated.h"

UCLASS(Blueprintable)
class TESTGAME_API UMCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	void InitCheatManager() override;
};

void UMCheatManager::InitCheatManager()
{
	for (TFieldIterator<UFunction> Iterator(GetClass(), EFieldIteratorFlags::ExcludeSuper); Iterator; ++Iterator)
	{
		UFunction* Function = *Iterator;

		if (Function && !Function->HasAnyFunctionFlags(FUNC_Native))
		{
			Function->FunctionFlags = FUNC_Exec | FUNC_BlueprintCallable;
		}
	}

	Super::InitCheatManager();
}
