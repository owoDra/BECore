// Copyright Eigi Chin

#include "BEInventoryItemDefinition.h"

#include "Fragment/BEInventoryItemFragment.h"
#include "BELogChannels.h"

#include "UObject/Class.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEInventoryItemDefinition)


UBEInventoryItemDefinition::UBEInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UBEInventoryItemFragment* UBEInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UBEInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UBEInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

bool UBEInventoryItemDefinition::IsDefinitionValid() const
{
	if (DisplayName == NAME_None)
	{
		UE_LOG(LogBEInventorySystem, Warning, TEXT("Invalid Display Name for [%s]."), *GetNameSafe(this));
		return false;
	}

	if (MaxStack <= 0)
	{
		UE_LOG(LogBEInventorySystem, Warning, TEXT("Invalid Max Stack for [%s]. Must be greater than or equal to 1."), *GetNameSafe(this));
		return false;
	}

	if (!CategoryTag.IsValid())
	{
		UE_LOG(LogBEInventorySystem, Warning, TEXT("Invalid Category Tag for [%s]."), *GetNameSafe(this));
		return false;
	}

	return true;
}
