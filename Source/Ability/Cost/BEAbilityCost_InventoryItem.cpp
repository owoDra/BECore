// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEAbilityCost_InventoryItem.h"
#include "Ability/Abilities/BEGameplayAbility.h"
#include "Inventory/BEInventoryManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAbilityCost_InventoryItem)


UBEAbilityCost_InventoryItem::UBEAbilityCost_InventoryItem()
{
	Quantity.SetValue(1.0f);
}

bool UBEAbilityCost_InventoryItem::CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
#if 0
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (UBEInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<UBEInventoryManagerComponent>())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

			return InventoryComponent->GetTotalItemCountByDefinition(ItemDefinition) >= NumItemsToConsume;
		}
	}
#endif
	return false;
}

void UBEAbilityCost_InventoryItem::ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
#if 0
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (UBEInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<UBEInventoryManagerComponent>())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

				InventoryComponent->ConsumeItemsByDefinition(ItemDefinition, NumItemsToConsume);
			}
		}
	}
#endif
}
