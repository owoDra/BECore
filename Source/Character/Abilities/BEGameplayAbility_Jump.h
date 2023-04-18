// Copyright Eigi Chin

#pragma once

#include "Ability/Abilities/BEGameplayAbility.h"

#include "Settings/BESettingsShared.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_Jump.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_Jump
 *
 *	キャラクターのジャンプを行うクラス
 */
UCLASS(Abstract)
class UBEGameplayAbility_Jump : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
	void CharacterJumpStop();
};
