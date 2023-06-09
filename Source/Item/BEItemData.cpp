// Copyright Eigi Chin

#include "Item/BEItemData.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEItemData)


#define LOCTEXT_NAMESPACE "BEItem"

#if WITH_EDITOR
EDataValidationResult UBEItemData::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (UBEItemDataFragment* Fragment : Fragments)
	{
		if (Fragment)
		{
			EDataValidationResult ChildResult = Fragment->IsDataValid(ValidationErrors);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("FragmentEntryIsNull", "Null entry at index {0} in Fragments"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	if (DisplayName.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		ValidationErrors.Add(FText::Format(LOCTEXT("FragmentInvalidDisplayName", "Invalid Display Name for [{0}]."), FText::AsCultureInvariant(GetNameSafe(this))));
	}

	// BP によって子クラスが作成されていないか検証(C++ -> BP は許可するが、BP -> BP は禁止)
	if (!GetClass()->IsNative())
	{
		UClass* ParentClass = GetClass()->GetSuperClass();

		// Find the native parent
		UClass* FirstNativeParent = ParentClass;
		while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
		{
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}

		if (FirstNativeParent != ParentClass)
		{
			ValidationErrors.Add(FText::Format(LOCTEXT("ItemInheritenceIsUnsupported", "Blueprint subclasses of Blueprint experiences is not currently supported. Parent class was {0} but should be {1}."),
				FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
				FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
			));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UBEItemData::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UBEItemDataFragment* Fragment : Fragments)
	{
		if (Fragment)
		{
			Fragment->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

const UBEItemDataFragment* UBEItemData::FindFragmentByClass(TSubclassOf<UBEItemDataFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UBEItemDataFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
