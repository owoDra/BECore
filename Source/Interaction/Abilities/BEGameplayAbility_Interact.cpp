// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameplayAbility_Interact.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Containers/EnumAsByte.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionStatics.h"
#include "Interaction/Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "Player/BEPlayerController.h"
#include "UI/IndicatorSystem/IndicatorDescriptor.h"
#include "UI/IndicatorSystem/BEIndicatorManagerComponent.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/ScriptInterface.h"
#include "BEGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbility_Interact)

class UUserWidget;


UBEGameplayAbility_Interact::UBEGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EBEAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBEGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantNearbyInteraction* Task = UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(this, InteractionScanRange, InteractionScanRate);
		Task->ReadyForActivation();
	}
}

void UBEGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
{
	if (ABEPlayerController* PC = GetBEPlayerControllerFromActorInfo())
	{
		if (UBEIndicatorManagerComponent* IndicatorManager = UBEIndicatorManagerComponent::GetComponent(PC))
		{
			for (UIndicatorDescriptor* Indicator : Indicators)
			{
				IndicatorManager->RemoveIndicator(Indicator);
			}
			Indicators.Reset();

			for (const FInteractionOption& InteractionOption : InteractiveOptions)
			{
				AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

				TSoftClassPtr<UUserWidget> InteractionWidgetClass = 
					InteractionOption.InteractionWidgetClass.IsNull() ? DefaultInteractionWidgetClass : InteractionOption.InteractionWidgetClass;

				UIndicatorDescriptor* Indicator = NewObject<UIndicatorDescriptor>();
				Indicator->SetDataObject(InteractableTargetActor);
				Indicator->SetSceneComponent(InteractableTargetActor->GetRootComponent());
				Indicator->SetIndicatorClass(InteractionWidgetClass);
				IndicatorManager->AddIndicator(Indicator);

				Indicators.Add(Indicator);
			}
		}
		else
		{
			//TODO This should probably be a noisy warning.  Why are we updating interactions on a PC that can never do anything with them?
		}
	}

	CurrentOptions = InteractiveOptions;
}

void UBEGameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		const FInteractionOption& InteractionOption = CurrentOptions[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
		// that only the actor knows.
		FGameplayEventData Payload;
		Payload.EventTag = TAG_Event_InteractionActivate;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		// If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
		// may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
		// door actor.
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(TAG_Event_InteractionActivate, Payload);

		// Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
		// source InteractableTarget actor as the owner actor.
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// The actor info needed for the interaction.
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// Trigger the ability using event tag.
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			TAG_Event_InteractionActivate,
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);
	}
}
