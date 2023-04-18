// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

class UBEInventoryItemInstance;


/**
 * UBEInventoryItemFragment
 * 
 * Inventory Item に付与できる追加情報
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBEInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UBEInventoryItemInstance* Instance) const {}

};