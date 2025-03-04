#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
};