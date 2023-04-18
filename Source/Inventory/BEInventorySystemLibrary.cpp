// Copyright Eigi Chin

#include "BEInventorySystemLibrary.h"

#include "BEInventoryItemDefinition.h"
#include "Fragment/BEInventoryItemFragment.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEInventorySystemLibrary)


const UBEInventoryItemFragment* UBEInventorySystemLibrary::FindItemDefinitionFragment(TSubclassOf<UBEInventoryItemDefinition> ItemDef, TSubclassOf<UBEInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UBEInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

const bool UBEInventorySystemLibrary::IsItemDefinitionValid(TSubclassOf<UBEInventoryItemDefinition> ItemDef)
{
	if (ItemDef != nullptr)
	{
		return GetDefault<UBEInventoryItemDefinition>(ItemDef)->IsDefinitionValid();
	}
	return false;
}
