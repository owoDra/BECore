// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

class UBEInventoryItemInstance;


/**
 * UBEInventoryItemFragment
 * 
 * Inventory Item Ç…ïtó^Ç≈Ç´ÇÈí«â¡èÓïÒ
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBEInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UBEInventoryItemInstance* Instance) const {}

};