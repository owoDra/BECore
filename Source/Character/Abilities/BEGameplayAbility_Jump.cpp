// Copyright Eigi Chin

#include "BEGameplayAbility_Jump.h"

#include "Character/BECharacter.h"
#include "Player/BELocalPlayer.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_Jump)


UBEGameplayAbility_Jump::UBEGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UBEGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

void UBEGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBEGameplayAbility_Jump::CharacterJumpStart()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled() && !BECharacter->bPressedJump)
		{
			BECharacter->Jump();
		}
	}
}

void UBEGameplayAbility_Jump::CharacterJumpStop()
{
	if (ABECharacter* BECharacter = GetBECharacterFromActorInfo())
	{
		if (BECharacter->IsLocallyControlled() && BECharacter->bPressedJump)
		{
			BECharacter->StopJumping();
		}
	}
}
