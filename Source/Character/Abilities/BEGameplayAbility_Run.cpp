// Copyright Eigi Chin

#include "BEGameplayAbility_Run.h"

#include "Character/BECharacter.h"
#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_Run)


UBEGameplayAbility_Run::UBEGameplayAbility_Run(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBEGameplayAbility_Run::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

void UBEGameplayAbility_Run::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

bool UBEGameplayAbility_Run::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

void UBEGameplayAbility_Run::HandleSettingChanged(UBESettingsShared* InSettings)
{
	if (InSettings)
	{
		UseToggleRun = InSettings->GetUseToggleRun();
		DefaultRun = InSettings->GetDefaultRun();

		if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
		{
			if (BECharacter->IsLocallyControlled())
			{
				if (DefaultRun)
				{
					BECharacter->Run();
				}
				else
				{
					BECharacter->UnRun();
				}
			}
		}
	}
}

void UBEGameplayAbility_Run::CharacterRunStart()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			if (DefaultRun)
			{
				BECharacter->UnRun();
			}
			else
			{
				BECharacter->Run();
			}
		}
	}
}

void UBEGameplayAbility_Run::CharacterRunStop()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			if (DefaultRun)
			{
				BECharacter->Run();
			}
			else
			{
				BECharacter->UnRun();
			}
		}
	}
}

bool UBEGameplayAbility_Run::ShouldToggleRun() const
{
	return UseToggleRun;
}
