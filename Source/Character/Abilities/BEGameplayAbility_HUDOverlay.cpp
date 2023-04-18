// Copyright Eigi Chin

#include "BEGameplayAbility_HUDOverlay.h"

#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_HUDOverlay)


UBEGameplayAbility_HUDOverlay::UBEGameplayAbility_HUDOverlay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBEGameplayAbility_HUDOverlay::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActorInfo->IsLocallyControlled())
	{
		if (UBELocalPlayer* BELP = Cast<UBELocalPlayer>(ActorInfo->PlayerController->GetLocalPlayer()))
		{
			if (UBESettingsShared* Settings = BELP->GetSharedSettings())
			{
				SettingChangeDelegate = Settings->OnSettingApplied.AddUObject(this, &ThisClass::HandleSettingChanged);
				HandleSettingChanged(Settings);
			}
		}
	}
}

void UBEGameplayAbility_HUDOverlay::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (ActorInfo->IsLocallyControlled())
	{
		if (UBELocalPlayer* BELP = Cast<UBELocalPlayer>(ActorInfo->PlayerController->GetLocalPlayer()))
		{
			if (UBESettingsShared* Settings = BELP->GetSharedSettings())
			{
				Settings->OnSettingChanged.Remove(SettingChangeDelegate);
			}
		}
	}
}

bool UBEGameplayAbility_HUDOverlay::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UBEGameplayAbility_HUDOverlay::HandleSettingChanged(UBESettingsShared* InSettings)
{
	if (InSettings)
	{
		UseToggleOverlay = InSettings->GetUseToggleOverlay();
	}
}

bool UBEGameplayAbility_HUDOverlay::ShouldToggleOverlay() const
{
	return UseToggleOverlay;
}
