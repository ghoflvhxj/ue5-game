#include "MGameInstance.h"
#include "Blueprint/UserWidget.h"

#include "TestGame/MItem/Drop.h"
#include "TestGame/MItem/ItemBase.h"
#include "TestGame/MCharacter/MMonster.h"
#include "TestGame/MAbility/MActionStruct.h"
#include "TestGame/MCharacter/MPlayer.h"
#include "TestGame/MFunctionLibrary/MMiscFunctionLibrary.h"
#include "TestGame/MAbility/MEffect.h"
#include "TestGame/MWeapon/Weapon.h"
#include "SkillSubsystem.h"

void UMGameInstance::Init()
{
	Super::Init();

	if (IsValid(ItemTable))
	{
		//TArray<FItemBaseInfo*> ItemBaseInfos;
		//ItemDataTable->GetAllRows(TEXT("ItemTable"), ItemBaseInfos);

		//for (const FItemBaseInfo* const ItemBaseInfo : ItemBaseInfos)
		//{
		//	ItemIndexToNameMap.Emplace(ItemBaseInfo->Index, ItemBaseInfo->);
		//}
	}
}


void UMGameInstance::OnStart()
{
	Super::OnStart();

	if (IsValid(SkillTable))
	{
		TArray<FSkillTableRow*> SkillTableRows;
		SkillTable->GetAllRows(TEXT("SkillTable"), SkillTableRows);
		for (FSkillTableRow* SkilTableRow : SkillTableRows)
		{
			UMMiscFunctionLibrary::LoadAssetFrom(FSkillTableRow::StaticStruct(), SkilTableRow);
		}
	}

	UE_CLOG(GetWorld()->IsNetMode(NM_DedicatedServer) == false, LogTemp, Warning, TEXT("ghoflvhxj %s"), *FString(__FUNCTION__));
}

void UMGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);
	UE_CLOG(GetWorld()->IsNetMode(NM_DedicatedServer) == false, LogTemp, Warning, TEXT("ghoflvhxj LoadComplete %s"), *MapName);
}

const FMonsterTableRow& UMGameInstance::GetMonsterTableRow(int32 InIndex)
{
	if (FMonsterTableRow* TableRow = GetTableRow<FMonsterTableRow>(MonsterTable, InIndex))
	{
		return *TableRow;
	}

	return FMonsterTableRow::Empty;
}

const FWeaponData& UMGameInstance::GetWeaponTableRow(UObject* Context, int32 InIndex)
{
	const FWeaponData* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FWeaponData>(GameInstance->WeaponTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		static FWeaponData Empty;
		Out = &Empty;
	}

	return *Out;
}

void UMGameInstance::LoadSkillAsset(UObject* WorldContext, int32 InSkillIndex, bool bIncludeChildren)
{
	const FSkillTableRow& SkillTableRow = GetSkillTableRow(WorldContext, InSkillIndex);

	for (const FBuffInfo& BuffInfo : SkillTableRow.BuffInfos)
	{
		const FEffectTableRow& EffectTableRow = GetEffectTableRow(WorldContext, BuffInfo.EffectIndex);
		UMMiscFunctionLibrary::LoadAssetFrom(FEffectTableRow::StaticStruct(), &EffectTableRow);
	}
}

const FSkillTableRow& UMGameInstance::GetSkillTableRow(UObject* Context, int32 InIndex)
{
	const FSkillTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FSkillTableRow>(GameInstance->SkillTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FSkillTableRow::Empty;
	}

	return *Out;
}

TArray<FSkillTableRow> UMGameInstance::GetSkillTableRowsByPredicate(TFunction<bool(const FSkillTableRow&)> Pred)
{
	TArray<FSkillTableRow*> SkillTableRows;
	if (IsValid(SkillTable))
	{
		SkillTable->GetAllRows<FSkillTableRow>(TEXT("SkillTable"), SkillTableRows);
	}

	TArray<FSkillTableRow> OutRows;
	for (FSkillTableRow* SkillTableRow : SkillTableRows)
	{
		if (Pred(*SkillTableRow))
		{
			OutRows.Add(*SkillTableRow);
		}
	}

	return OutRows;
}

TArray<int32> UMGameInstance::GetSkillEnhanceTableRowsByPredicate(TFunction<bool(const FSkillEnhanceTableRow&)> Pred)
{
	TArray<FSkillEnhanceTableRow*> SkillTableRows;
	if (IsValid(SkillEnhanceTable))
	{
		SkillEnhanceTable->GetAllRows<FSkillEnhanceTableRow>(TEXT("SkillEnhanceTable"), SkillTableRows);
	}

	TArray<int32> OutRows;
	for (FSkillEnhanceTableRow* SkillTableRow : SkillTableRows)
	{
		if (Pred(*SkillTableRow))
		{
			OutRows.Add(SkillTableRow->Index);
		}
	}

	return OutRows;
}

const FEffectTableRow& UMGameInstance::GetEffectTableRow(const UObject* WorldContextObject, int32 InIndex)
{
	static FEffectTableRow Empty;

	const FEffectTableRow* Out = nullptr;
	if (IsValid(WorldContextObject))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
		{
			Out = GameInstance->GetTableRow<FEffectTableRow>(GameInstance->EffectTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &Empty;
	}

	return *Out;
}

const FActionTableRow& UMGameInstance::GetActionTableRow(int32 InIndex)
{
	if (FActionTableRow* TableRow = GetTableRow<FActionTableRow>(ActionTable, InIndex))
	{
		return *TableRow;
	}

	return FActionTableRow::Empty;
}

TArray<FGameItemTableRow*> UMGameInstance::FilterItemByPredicate(TFunction<bool(const FGameItemTableRow* const InGameItemTableRow)> Func)
{
	TArray<FGameItemTableRow*> GameItemTableRows;
	ItemTable->GetAllRows<FGameItemTableRow>(TEXT("ItemTable"), GameItemTableRows);

	GameItemTableRows.FilterByPredicate(Func);

	return GameItemTableRows;
}

void UMGameInstance::IterateItemTable(TFunction<void(const FGameItemTableRow& GameItemTableRow)> Function)
{
	if (IsValid(ItemTable) == false)
	{
		return;
	}

	TArray<FGameItemTableRow*> GameItemTableRows;
	ItemTable->GetAllRows<FGameItemTableRow>(TEXT("ItemTable"), GameItemTableRows);

	for (FGameItemTableRow* GameItemTableRow : GameItemTableRows)
	{
		Function(*GameItemTableRow);
	}
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(int32 InIndex)
{
	return GetItemTableRow(*FString::FromInt(InIndex));
}

FGameItemTableRow* UMGameInstance::GetItemTableRow(FName InRowName)
{
	if (IsValid(ItemTable) && InRowName != NAME_None)
	{
		return ItemTable->FindRow<FGameItemTableRow>(InRowName, TEXT("ItemTable"));
	}

	return nullptr;
}

const FDropTableRow& UMGameInstance::GetDropTableRow(UObject* Context, int32 InIndex)
{
	const FDropTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FDropTableRow>(GameInstance->DropTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FDropTableRow::Empty;
	}

	return *Out;
}

const FSkillEnhanceTableRow& UMGameInstance::GetSkillEnhanceTableRow(int32 InIndex)
{
	if (FSkillEnhanceTableRow* TableRow = GetTableRow<FSkillEnhanceTableRow>(SkillEnhanceTable, InIndex))
	{
		return *TableRow;
	}

	return FSkillEnhanceTableRow::Empty;
}

const FPlayerCharacterTableRow& UMGameInstance::GetPlayerCharacterTableRow(UObject* Context, int32 InIndex)
{
	const FPlayerCharacterTableRow* Out = nullptr;
	if (IsValid(Context))
	{
		if (UMGameInstance* GameInstance = Cast<UMGameInstance>(UGameplayStatics::GetGameInstance(Context)))
		{
			Out = GameInstance->GetTableRow<FPlayerCharacterTableRow>(GameInstance->PlayerCharacterTable, InIndex);
		}
	}

	if (Out == nullptr)
	{
		Out = &FPlayerCharacterTableRow::Empty;
	}
	
	return *Out;
}

void UMGameInstance::OpenLevel(FName InLevelName)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	if (UUserWidget* NewWidget = CreateWidget(this, LoadWidgetClass))
	{
		LoadWidget = NewWidget;
		LoadWidget->AddToViewport(0);
	}
	
	// 이건 단순히 궁금쓰해서 로그 넣어봄
	if (World->IsNetMode(NM_Standalone))
	{
		UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj OpenLevel Standalone"));
	}
	else if (World->IsNetMode(NM_Client))
	{
		UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj OpenLevel Client"));
	}

	auto OpenLevelLambda = [this, InLevelName]() {
		UGameplayStatics::OpenLevel(this, InLevelName);
	};

	if (InLevelName.ToString().Contains(UMMiscFunctionLibrary::GetServerIpAddress()))
	{
		OpenLevelLambda();
	}
	else
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OpenLevelLambda]() {
			OpenLevelLambda();
		}));
	}
}

//void UMGameInstance::ShowLoadingWidget()
//{
//	if (UUserWidget* NewWidget = CreateWidget(this, LoadingWidgetClass))
//	{
//		LoadingWidget = NewWidget;
//	}
//}
