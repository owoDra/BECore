// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameplayAbility_FromEquipment.h"
#include "BEEquipmentInstance.h"
#include "Inventory/BEInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_FromEquipment)


UBEGameplayAbility_FromEquipment::UBEGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UBEEquipmentInstance* UBEGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UBEEquipmentInstance>(Spec->SourceObject);
	}

	return nullptr;
}

UBEInventoryItemInstance* UBEGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	if (UBEEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UBEInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}


#if WITH_EDITOR
EDataValidationResult UBEGameplayAbility_FromEquipment::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		ValidationErrors.Add(NSLOCTEXT("BE", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif