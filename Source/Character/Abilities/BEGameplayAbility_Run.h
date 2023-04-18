// Copyright Eigi Chin

#pragma once

#include "Ability/Abilities/BEGameplayAbility.h"

#include "Settings/BESettingsShared.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_Run.generated.h"

class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_Run
 *
 *	キャラクターの走りを行うアビリティ
 */
UCLASS(Abstract)
class UBEGameplayAbility_Run : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_Run(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
		virtual void HandleSettingChanged(UBESettingsShared* InSettings);

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterRunStart();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		void CharacterRunStop();

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
		bool ShouldToggleRun() const;

private:
	FDelegateHandle SettingChangeDelegate;

	bool UseToggleRun = false;

	bool DefaultRun = false;
};
