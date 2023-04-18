// Copyright Eigi Chin

#pragma once

#include "Ability/Abilities/BEGameplayAbility.h"

#include "Settings/BESettingsShared.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_HUDOverlay.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_HUDOverlay
 *
 *	HUD Overlayを表示するアビリティ(スコアボードなど)
 */
UCLASS(Abstract)
class UBEGameplayAbility_HUDOverlay : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_HUDOverlay(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
		virtual void HandleSettingChanged(UBESettingsShared* InSettings);

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		bool ShouldToggleOverlay() const;

private:
	FDelegateHandle SettingChangeDelegate;

	bool UseToggleOverlay = false;
};
