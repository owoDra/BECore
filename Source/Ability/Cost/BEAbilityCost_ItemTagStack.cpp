// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEAbilityCost_ItemTagStack.h"
#include "Ability/Abilities/BEGameplayAbility.h"
#include "Equipment/BEGameplayAbility_FromEquipment.h"
#include "Inventory/BEInventoryItemInstance.h"
#include "BEGameplayTags.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Math/UnrealMathSSE.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAbilityCost_ItemTagStack)


UBEAbilityCost_ItemTagStack::UBEAbilityCost_ItemTagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_Ability_ActivateFail_Cost;
}

bool UBEAbilityCost_ItemTagStack::CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UBEGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UBEGameplayAbility_FromEquipment>(Ability))
	{
		if (UBEInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
			const bool bCanApplyCost = ItemInstance->GetStatTagStackCount(Tag) >= NumStacks;

			// Inform other abilities why this cost cannot be applied
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}
			return bCanApplyCost;
		}
	}
	return false;
}

void UBEAbilityCost_ItemTagStack::ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UBEGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UBEGameplayAbility_FromEquipment>(Ability))
		{
			if (UBEInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				ItemInstance->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
