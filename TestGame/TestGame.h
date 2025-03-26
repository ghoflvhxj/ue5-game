// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/* NetMode 출력을 포함 해줌 */
#define UE_NLOG(Category, Verbosity, Format, ...) \
{\
	const ENetMode NetMode = (GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone); \
	const TCHAR* Prefix = (NetMode == NM_Client) ? TEXT("[Client] ") : ((NetMode == NM_DedicatedServer) ? TEXT("[Server] ") : TEXT("[Standalone] ")); \
	UE_LOG(Category, Verbosity, TEXT("%s" Format), Prefix, ##__VA_ARGS__); \
}



DECLARE_LOG_CATEGORY_EXTERN(LogGAS, Error, All);