// Copyright owoDra

#include "BEAbilityCost_EquipmentTagStack.h"

#include "Item/Equipment/BEGameplayAbility_FromEquipment.h"
#include "Item/Equipment/BEEquipmentInstance.h"
#include "Ability/BEGameplayAbility.h"
#include "GameplayTag/BETags_GameplayAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Math/UnrealMathSSE.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAbilityCost_EquipmentTagStack)


UBEAbilityCost_EquipmentTagStack::UBEAbilityCost_EquipmentTagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_Ability_ActivateFail_Cost;
}

bool UBEAbilityCost_EquipmentTagStack::CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UBEGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UBEGameplayAbility_FromEquipment>(Ability))
	{
		if (UBEEquipmentInstance* Instance = EquipmentAbility->GetAssociatedEquipment())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);
			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
			const bool bCanApplyCost = Instance->GetStatTagStackCount(Tag) >= NumStacks;

			// このコストを適用できない理由を他の Ability に知らせる
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}
			return bCanApplyCost;
		}
	}
	return false;
}

void UBEAbilityCost_EquipmentTagStack::ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UBEGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UBEGameplayAbility_FromEquipment>(Ability))
		{
			if (UBEEquipmentInstance* Instance = EquipmentAbility->GetAssociatedEquipment())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				Instance->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
