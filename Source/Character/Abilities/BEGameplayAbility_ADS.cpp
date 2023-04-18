// Copyright Eigi Chin

#include "BEGameplayAbility_ADS.h"

#include "Character/BECharacter.h"
#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_ADS)


UBEGameplayAbility_ADS::UBEGameplayAbility_ADS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBEGameplayAbility_ADS::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

void UBEGameplayAbility_ADS::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

bool UBEGameplayAbility_ADS::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const ABECharacter* BECharacter = Cast<ABECharacter>(ActorInfo->AvatarActor.Get());
	if (!BECharacter)
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UBEGameplayAbility_ADS::HandleSettingChanged(UBESettingsShared* InSettings)
{
	if (InSettings)
	{
		UseToggleADS = InSettings->GetUseToggleADS();
	}
}

void UBEGameplayAbility_ADS::CharacterAimStart()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			BECharacter->Aim();
		}
	}
}

void UBEGameplayAbility_ADS::CharacterAimStop()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			BECharacter->UnAim();
		}
	}
}

bool UBEGameplayAbility_ADS::ShouldToggleADS() const
{
	return UseToggleADS;
}
