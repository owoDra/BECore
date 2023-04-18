// Copyright Eigi Chin

#pragma once

#include "Ability/Abilities/BEGameplayAbility.h"

#include "Settings/BESettingsShared.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_Crouch.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_Crouch
 *
 *	キャラクターのしゃがみをするアビリティ
 */
UCLASS(Abstract)
class UBEGameplayAbility_Crouch : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_Crouch(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
		virtual void HandleSettingChanged(UBESettingsShared* InSettings);

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterCrouchStart();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterCrouchStop();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		bool ShouldToggleCrouch() const;

private:
	FDelegateHandle SettingChangeDelegate;

	bool UseToggleCrouch = false;
};
