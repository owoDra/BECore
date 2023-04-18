// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Inventory/BEInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "InventoryFragment_EquippableItem.generated.h"

class UBEEquipmentDefinition;
class UObject;


UCLASS()
class UInventoryFragment_EquippableItem : public UBEInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=BE)
	TSubclassOf<UBEEquipmentDefinition> EquipmentDefinition;
};
