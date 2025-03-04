#include "MMiscFunctionLibrary.h"
#include "Online/CoreOnline.h"


bool UMMiscFunctionLibrary::CompareUniqueNetID(APlayerState* Lhs, APlayerState* Rhs)
{
	if (IsValid(Lhs) && IsValid(Rhs))
	{
		return Lhs->GetUniqueId().GetUniqueNetId() == Rhs->GetUniqueId().GetUniqueNetId();
	}

	return false;
}

FString UMMiscFunctionLibrary::GetServerIpAddress()
{
	FString ConfigFilePath = FPaths::ProjectDir() / TEXT("ServerAddress.txt");
	FString ServerAddress = TEXT("127.0.0.1");

	if (FFileHelper::LoadFileToString(ServerAddress, *ConfigFilePath))
	{
		ServerAddress = ServerAddress.TrimStartAndEnd();
		return ServerAddress;
	}

	return ServerAddress;
}
