// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "Ability/Phases/BEGamePhaseSubsystem.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Abilities/BEGameplayAbility.h"
#include "Ability/Phases/BEGamePhaseAbility.h"
#include "BEGamePhaseLog.h"

#include "Containers/UnrealString.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagsManager.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "Templates/Tuple.h"
#include "Trace/Detail/Channel.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGamePhaseSubsystem)

class UBEGameplayAbility;
class UObject;

DEFINE_LOG_CATEGORY(LogBEGamePhase);


//////////////////////////////////////////////////////////////////////
// UBEGamePhaseSubsystem

UBEGamePhaseSubsystem::UBEGamePhaseSubsystem()
{
}

void UBEGamePhaseSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

bool UBEGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		return true;
	}

	return false;
}

bool UBEGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UBEGamePhaseSubsystem::StartPhase(TSubclassOf<UBEGamePhaseAbility> PhaseAbility, FBEGamePhaseDelegate PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UBEAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UBEAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);
		
		if (FoundSpec && FoundSpec->IsActive())
		{
			FBEGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UBEGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UBEGamePhaseAbility> PhaseAbility, const FBEGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
	const FBEGamePhaseDelegate EndedDelegate = FBEGamePhaseDelegate::CreateWeakLambda(const_cast<UObject*>(PhaseEndedDelegate.GetUObject()), [PhaseEndedDelegate](const UBEGamePhaseAbility* PhaseAbility) {
		PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
	});

	StartPhase(PhaseAbility, EndedDelegate);
}

void UBEGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FBEGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FBEGamePhaseTagDelegate ActiveDelegate = FBEGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag) {
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UBEGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FBEGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FBEGamePhaseTagDelegate EndedDelegate = FBEGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag) {
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UBEGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FBEGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UBEGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FBEGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

bool UBEGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FBEGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

void UBEGamePhaseSubsystem::OnBeginPhase(const UBEGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();
	const FGameplayTag IncomingPhaseParentTag = UGameplayTagsManager::Get().RequestGameplayTagDirectParent(IncomingPhaseTag);

	UE_LOG(LogBEGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UBEAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UBEAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const UBEGamePhaseAbility* ActivePhaseAbility = CastChecked<UBEGamePhaseAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();
			
			// So if the active phase currently matches the incoming phase tag, we allow it.
			// i.e. multiple gameplay abilities can all be associated with the same phase tag.
			// For example,
			// You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
			// Game.Playing phase will still be active, and if someone were to push another one, like,
			// Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
			// continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
			if (!ActivePhaseTag.MatchesTag(IncomingPhaseTag) && ActivePhaseTag.MatchesTag(IncomingPhaseParentTag))
			{
				UE_LOG(LogBEGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UBEGameplayAbility* BEAbility, FGameplayAbilitySpecHandle Handle) {
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FBEGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// Notify all observers of this phase that it has started.
		for (const FPhaseObserver& Observer : PhaseStartObservers)
		{
			if (Observer.IsMatch(IncomingPhaseTag))
			{
				Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

void UBEGamePhaseSubsystem::OnEndPhase(const UBEGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogBEGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FBEGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// Notify all observers of this phase that it has ended.
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool UBEGamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch(MatchType)
	{
	case EPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case EPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}