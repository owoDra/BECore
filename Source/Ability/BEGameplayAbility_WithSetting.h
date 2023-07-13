// Copyright owoDra

#pragma once

#include "BEGameplayAbility.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_WithSetting.generated.h"

class UObject;
class UBEGameSharedSettings;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UBEGameplayAbility_WithSetting
 *
 *	設定との紐づけを行うアビリティ
 */
UCLASS(Abstract)
class BECORE_API UBEGameplayAbility_WithSetting : public UBEGameplayAbility
{
	GENERATED_BODY()
public:
	UBEGameplayAbility_WithSetting(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	UFUNCTION() virtual void HandleSettingChanged(UBEGameSharedSettings* InSettings) {}

private:
	FDelegateHandle SettingChangeDelegate;
};
