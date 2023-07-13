// Copyright owoDra

#pragma once

#include "Item/BEItemData.h"

#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Stackable.generated.h"


UCLASS()
class UBEItemDataFragment_Stackable : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStack;
};
