// Copyright Eigi Chin

#include "BEItemFunctionLibrary.h"

#include "BEItemData.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEItemFunctionLibrary)


const UBEItemDataFragment* UBEItemFunctionLibrary::FindItemDefinitionFragment(const UBEItemData* ItemData, TSubclassOf<UBEItemDataFragment> FragmentClass)
{
	if (ItemData != nullptr)
	{
		return ItemData->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
