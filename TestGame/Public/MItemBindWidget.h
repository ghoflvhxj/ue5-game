#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MItemBindWidget.generated.h"

UCLASS()
class TESTGAME_API UMItemBindWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetItemIndex(int32 InIndex);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemIndex = INDEX_NONE;
};
