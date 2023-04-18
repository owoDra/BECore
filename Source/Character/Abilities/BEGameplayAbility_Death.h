// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Ability/Abilities/BEGameplayAbility.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_Death.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;


/**
 * UBEGameplayAbility_Death
 *
 *	キャラクターがデスした際に実行されるアビリティ
 *	Gameplay Event "Event.Death"によって自動的に実行される
 */
UCLASS(Abstract)
class UBEGameplayAbility_Death : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	void DoneAddingNativeTags();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Starts the death sequence.
	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
	void StartDeath();

	// Finishes the death sequence.
	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
	void FinishDeath();

protected:

	// If enabled, the ability will automatically call StartDeath.  FinishDeath is always called when the ability ends if the death was started.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BE|Death")
	bool bAutoStartDeath;
};
