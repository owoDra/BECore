// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameplayAbility_Death.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Character/BEHealthComponent.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "HAL/Platform.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "Trace/Detail/Channel.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_Death)


UBEGameplayAbility_Death::UBEGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bAutoStartDeath = true;

	UGameplayTagsManager::Get().CallOrRegister_OnDoneAddingNativeTagsDelegate(FSimpleDelegate::CreateUObject(this, &ThisClass::DoneAddingNativeTags));
}

void UBEGameplayAbility_Death::DoneAddingNativeTags()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = TAG_Event_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UBEGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UBEAbilitySystemComponent* BEASC = CastChecked<UBEAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(TAG_Ability_Behavior_SurvivesDeath);

	// Cancel all abilities and block others from starting.
	BEASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	if (!ChangeActivationGroup(EBEAbilityActivationGroup::Exclusive_Blocking))
	{
		UE_LOG(LogBEAbilitySystem, Error, TEXT("UBEGameplayAbility_Death::ActivateAbility: Ability [%s] failed to change activation group to blocking."), *GetName());
	}

	if (bAutoStartDeath)
	{
		StartDeath();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBEGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.
	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBEGameplayAbility_Death::StartDeath()
{
	if (UBEHealthComponent* HealthComponent = UBEHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EBEDeathState::NotDead)
		{
			HealthComponent->StartDeath();
		}
	}
}

void UBEGameplayAbility_Death::FinishDeath()
{
	if (UBEHealthComponent* HealthComponent = UBEHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EBEDeathState::DeathStarted)
		{
			HealthComponent->FinishDeath();
		}
	}
}
