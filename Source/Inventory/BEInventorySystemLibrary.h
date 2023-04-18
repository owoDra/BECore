// Copyright Eigi Chin

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEInventorySystemLibrary.generated.h"

class UBEInventoryItemFragment;
class UBEInventoryItemDefinition;


/**
 * BEInventorySystemLibrary
 * 
 * Inventory ŠÖŒW‚Ì‘€ì‚ğs‚¤ŠÖ”‚Ìƒ‰ƒCƒuƒ‰ƒŠ
 */
UCLASS()
class UBEInventorySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Inventory", meta=(DeterminesOutputType=FragmentClass))
	static const UBEInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UBEInventoryItemDefinition> ItemDef, TSubclassOf<UBEInventoryItemFragment> FragmentClass);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static const bool IsItemDefinitionValid(TSubclassOf<UBEInventoryItemDefinition> ItemDef);

};
