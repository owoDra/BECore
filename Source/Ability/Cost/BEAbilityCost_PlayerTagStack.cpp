// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEAbilityCost_PlayerTagStack.h"

#include "Player/BEPlayerState.h"
#include "Ability/Abilities/BEGameplayAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Controller.h"
#include "Math/UnrealMathSSE.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAbilityCost_PlayerTagStack)


UBEAbilityCost_PlayerTagStack::UBEAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool UBEAbilityCost_PlayerTagStack::CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (ABEPlayerState* PS = Cast<ABEPlayerState>(PC->PlayerState))
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return PS->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void UBEAbilityCost_PlayerTagStack::ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (ABEPlayerState* PS = Cast<ABEPlayerState>(PC->PlayerState))
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				PS->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
