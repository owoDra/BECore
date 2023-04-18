// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "BEAbilityCost.h"

#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "UObject/UObjectGlobals.h"

#include "BEAbilityCost_ItemTagStack.generated.h"


/**
* UBEAbilityCost_ItemTagStack
* 
*  Inventory Item のTag Stackを消費する種類のコスト
*/
UCLASS(meta=(DisplayName="Item Tag Stack"))
class UBEAbilityCost_ItemTagStack : public UBEAbilityCost
{
	GENERATED_BODY()

public:
	UBEAbilityCost_ItemTagStack();

	//~UBEAbilityCost interface
	virtual bool CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UBEAbilityCost interface

protected:
	/** How much of the tag to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** Which tag to spend some of */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;

	/** Which tag to send back as a response if this cost cannot be applied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag FailureTag;
};
