// Copyright Eigi Chin

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEItemFunctionLibrary.generated.h"

class UBEItemData;
class UBEItemDataFragment;


/**
 * UBEItemFunctionLibrary
 * 
 * Item �֌W�̑�����s���֐��̃��C�u����
 */
UCLASS()
class UBEItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Inventory", meta=(DeterminesOutputType=FragmentClass))
	static const UBEItemDataFragment* FindItemDefinitionFragment(const UBEItemData* ItemData, TSubclassOf<UBEItemDataFragment> FragmentClass);
};
