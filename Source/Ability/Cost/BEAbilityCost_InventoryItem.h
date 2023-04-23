// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "BEAbilityCost.h"

#include "GameplayAbilitySpec.h"
#include "ScalableFloat.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEAbilityCost_InventoryItem.generated.h"

class UBEGameplayAbility;
class UBEItemData;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
* ULyraAbilityCost_InventoryItem
* 
*  Inventory Itemの数量を消費する種類のコスト
*/
UCLASS(meta=(DisplayName="Inventory Item"))
class UBEAbilityCost_InventoryItem : public UBEAbilityCost
{
	GENERATED_BODY()

public:
	UBEAbilityCost_InventoryItem();

	//~UBEAbilityCost interface
	virtual bool CheckCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UBEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UBEAbilityCost interface

protected:
	/** How much of the item to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	FScalableFloat Quantity;

	/** Which item to consume */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	TSubclassOf<UBEItemData> ItemDefinition;
};
