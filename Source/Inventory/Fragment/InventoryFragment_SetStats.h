// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Inventory/BEInventoryItemDefinition.h"

#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "InventoryFragment_SetStats.generated.h"

class UBEInventoryItemInstance;
class UObject;
struct FGameplayTag;


UCLASS()
class UInventoryFragment_SetStats : public UBEInventoryItemFragment
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TMap<FGameplayTag, int32> InitialItemStats;

public:
	virtual void OnInstanceCreated(UBEInventoryItemInstance* Instance) const override;

	int32 GetItemStatByTag(FGameplayTag Tag) const;
};
