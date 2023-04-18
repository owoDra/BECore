// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "HAL/Platform.h"
#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEInventoryItemInstance.generated.h"

class UBEInventoryItemDefinition;
class UBEInventoryItemFragment;
struct FGameplayTag;


/**
 * UBEInventoryItemInstance
 * 
 * Inventory Ç…ìoò^Ç≥ÇÍÇΩ Item ÇÃÉfÅ[É^
 */
UCLASS(BlueprintType)
class UBEInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UBEInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	bool HasStatTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TSubclassOf<UBEInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UBEInventoryItemFragment* FindFragmentByClass(TSubclassOf<UBEInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

private:
	void SetItemDef(TSubclassOf<UBEInventoryItemDefinition> InDef);

	friend struct FBEInventoryList;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	UPROPERTY(Replicated)
	TSubclassOf<UBEInventoryItemDefinition> ItemDef;

private:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS
};
