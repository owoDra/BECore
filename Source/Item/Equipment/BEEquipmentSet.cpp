// Copyright owoDra

#include "BEEquipmentSet.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEEquipmentSet)


#define LOCTEXT_NAMESPACE "BEEquipment"

UBEEquipmentSet::UBEEquipmentSet()
{
}

#if WITH_EDITOR
EDataValidationResult UBEEquipmentSet::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

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

#undef LOCTEXT_NAMESPACE
