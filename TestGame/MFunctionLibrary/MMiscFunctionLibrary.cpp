#include "MMiscFunctionLibrary.h"
#include "Online/CoreOnline.h"
#include "MGameInstance.h"
#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MGameState/MGameStateInGame.h"

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
	FString ConfigFilePath = FPaths::Combine(TEXT("../../../"), TEXT("ServerAddress.txt"));
	FString ServerAddress = TEXT("127.0.0.1");

	if (FFileHelper::LoadFileToString(ServerAddress, *ConfigFilePath))
	{
		ServerAddress = ServerAddress.TrimStartAndEnd();
		return ServerAddress;
	}

	return ServerAddress;
}

UAudioComponent* UMMiscFunctionLibrary::PlayUISoundByEffectIndex(const UObject* WorldContextObject, int32 InEffectIndex)
{
	return PlayUISoundByEffectIndexWithConcurrency(WorldContextObject, InEffectIndex, nullptr);
}

UAudioComponent* UMMiscFunctionLibrary::PlayUISoundByEffectIndexWithConcurrency(const UObject* WorldContextObject, int32 InEffectIndex, USoundConcurrency* Concurrency)
{
	const FEffectTableRow& EffectTableRow = UMGameInstance::GetEffectTableRow(WorldContextObject, InEffectIndex);

	TSoftObjectPtr<USoundBase> ObjectPtr(EffectTableRow.Sound);
	if (USoundBase* Sound = ObjectPtr.LoadSynchronous())
	{
		return UGameplayStatics::SpawnSound2D(WorldContextObject, Sound, 1.f, 1.f, 0.f, Concurrency);
	}

	return nullptr;
}

void UMMiscFunctionLibrary::PlayBGM(const UObject* WorldContextObject, int32 InEffectIndex)
{
	if (WorldContextObject == nullptr)
	{
		return;
	}

	const FEffectTableRow& EffectTableRow = UMGameInstance::GetEffectTableRow(WorldContextObject, InEffectIndex);

	TSoftObjectPtr<USoundBase> ObjectPtr(EffectTableRow.Sound);
	USoundBase* Sound = ObjectPtr.LoadSynchronous();
	if (IsValid(Sound) == false)
	{
		return;
	}

	AMGameStateInGame* GameState = WorldContextObject->GetWorld()->GetGameState<AMGameStateInGame>();
	if (IsValid(GameState) == false)
	{
		return;
	}

	GameState->ChangeBGM(Sound);
}

float UMMiscFunctionLibrary::UnwindDegree(float InDegree)
{
	return FMath::UnwindDegrees(InDegree);
}
