// Copyright owoDra

#pragma once

#include "Item/BEItemData.h"

#include "Internationalization/Text.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Detail.generated.h"

class UTexture2D;


UCLASS()
class UBEItemDataFragment_Detail : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Discription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;
};
