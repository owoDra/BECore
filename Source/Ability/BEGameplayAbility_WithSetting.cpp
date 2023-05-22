// Copyright Eigi Chin

#include "BEGameplayAbility_WithSetting.h"

#include "GameSetting/BEGameSharedSettings.h"
#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_WithSetting)


UBEGameplayAbility_WithSetting::UBEGameplayAbility_WithSetting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}


void UBEGameplayAbility_WithSetting::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActorInfo->IsLocallyControlled())
	{
		if (UBELocalPlayer* BELP = Cast<UBELocalPlayer>(ActorInfo->PlayerController->GetLocalPlayer()))
		{
			if (UBEGameSharedSettings* Settings = BELP->GetSharedSettings())
			{
				SettingChangeDelegate = Settings->OnSettingApplied.AddUObject(this, &ThisClass::HandleSettingChanged);
				HandleSettingChanged(Settings);
			}
		}
	}
}

void UBEGameplayAbility_WithSetting::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (ActorInfo->IsLocallyControlled())
	{
		if (UBELocalPlayer* BELP = Cast<UBELocalPlayer>(ActorInfo->PlayerController->GetLocalPlayer()))
		{
			if (UBEGameSharedSettings* Settings = BELP->GetSharedSettings())
			{
				Settings->OnSettingChanged.Remove(SettingChangeDelegate);
			}
		}
	}
}

bool UBEGameplayAbility_WithSetting::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags, 
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
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
