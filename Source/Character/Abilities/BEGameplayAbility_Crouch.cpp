// Copyright Eigi Chin

#include "BEGameplayAbility_Crouch.h"

#include "Character/BECharacter.h"
#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_Crouch)


UBEGameplayAbility_Crouch::UBEGameplayAbility_Crouch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBEGameplayAbility_Crouch::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

void UBEGameplayAbility_Crouch::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
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

bool UBEGameplayAbility_Crouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const ABECharacter* BECharacter = Cast<ABECharacter>(ActorInfo->AvatarActor.Get());
	if (!BECharacter || !BECharacter->CanCrouch())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UBEGameplayAbility_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop Crouching in case the ability blueprint doesn't call it.
	CharacterCrouchStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBEGameplayAbility_Crouch::HandleSettingChanged(UBESettingsShared* InSettings)
{
	if (InSettings)
	{
		UseToggleCrouch = InSettings->GetUseToggleCrouch();
	}
}

void UBEGameplayAbility_Crouch::CharacterCrouchStart()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			BECharacter->Crouch();
		}
	}
}

void UBEGameplayAbility_Crouch::CharacterCrouchStop()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled())
		{
			BECharacter->UnCrouch();
		}
	}
}

bool UBEGameplayAbility_Crouch::ShouldToggleCrouch() const
{
	return UseToggleCrouch;
}
