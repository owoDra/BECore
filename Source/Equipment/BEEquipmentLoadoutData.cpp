// Copyright Eigi Chin

#include "BEEquipmentLoadoutData.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEEquipmentLoadoutData)


#define LOCTEXT_NAMESPACE "BEEquipment"

UBEEquipmentLoadoutData::UBEEquipmentLoadoutData()
{
}

#if WITH_EDITOR
EDataValidationResult UBEEquipmentLoadoutData::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	// BP �ɂ���Ďq�N���X���쐬����Ă��Ȃ�������(C++ -> BP �͋����邪�ABP -> BP �͋֎~)
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
