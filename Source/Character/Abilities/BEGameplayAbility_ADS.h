// Copyright Eigi Chin

#pragma once

#include "Equipment/BEGameplayAbility_FromEquipment.h"

#include "Settings/BESettingsShared.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_ADS.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_ADS
 *
 *	ADSをするアビリティ
 */
UCLASS(Abstract)
class UBEGameplayAbility_ADS : public UBEGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_ADS(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
		virtual void HandleSettingChanged(UBESettingsShared* InSettings);

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterAimStart();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterAimStop();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		bool ShouldToggleADS() const;

private:
	FDelegateHandle SettingChangeDelegate;

	bool UseToggleADS = false;
};
