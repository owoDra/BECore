// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEPlayerState.h"

#include "Ability/Attributes/BECombatSet.h"
#include "Ability/Attributes/BEHealthSet.h"
#include "Ability/Attributes/BEMovementSet.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Character/BEPawnData.h"
#include "Character/BEPawnExtensionComponent.h"
#include "GameModes/BEExperienceManagerComponent.h"
#include "GameModes/BEGameMode.h"
#include "BELogChannels.h"
#include "BEPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Delegates/Delegate.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Trace/Detail/Channel.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

const FName ABEPlayerState::NAME_BEAbilityReady("BEAbilitiesReady");


ABEPlayerState::ABEPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EBEPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UBEAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UBEHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UBECombatSet>(TEXT("CombatSet"));
	MovementSet = CreateDefaultSubobject<UBEMovementSet>(TEXT("MovementSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;

	MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;
}

void ABEPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ABEPlayerState::Reset()
{
	Super::Reset();
}

void ABEPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void ABEPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void ABEPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
		case EBEPlayerConnectionType::Player:
		case EBEPlayerConnectionType::InactivePlayer:
			//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
			// (e.g., for long running servers where they might build up if lots of players cycle through)
			bDestroyDeactivatedPlayerState = true;
			break;
		default:
			bDestroyDeactivatedPlayerState = true;
			break;
	}
	
	SetPlayerConnectionType(EBEPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void ABEPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EBEPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EBEPlayerConnectionType::Player);
	}
}

void ABEPlayerState::OnExperienceLoaded(const UBEExperienceDefinition* /*CurrentExperience*/)
{
	if (ABEGameMode* BEGameMode = GetWorld()->GetAuthGameMode<ABEGameMode>())
	{
		if (const UBEPawnData* NewPawnData = BEGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogBE, Error, TEXT("ABEPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void ABEPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);
}

ABEPlayerController* ABEPlayerState::GetBEPlayerController() const
{
	return Cast<ABEPlayerController>(GetOwner());
}

UAbilitySystemComponent* ABEPlayerState::GetAbilitySystemComponent() const
{
	return GetBEAbilitySystemComponent();
}

void ABEPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	if (GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UBEExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UBEExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnBEExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void ABEPlayerState::SetPawnData(const UBEPawnData* InPawnData, bool Override)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		// PawnDataが既に設定されている場合に上書き
		if (Override)
		{
			PawnDataAbilityHandles.TakeFromAbilitySystem(AbilitySystemComponent);
		}

		// PawnDataが既に設定されている場合に処理を中断
		else
		{
			UE_LOG(LogBE, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
			return;
		}
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UBEAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &PawnDataAbilityHandles);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_BEAbilityReady);
	
	ForceNetUpdate();
}

void ABEPlayerState::K2_SetPawnData(const UBEPawnData* InPawnData, bool Override)
{
	SetPawnData(InPawnData, Override);
}

void ABEPlayerState::OnRep_PawnData()
{
}

void ABEPlayerState::SetPlayerConnectionType(EBEPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

void ABEPlayerState::SetSquadID(int32 NewSquadId)
{
	if (HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

		MySquadID = NewSquadId;
	}
}

void ABEPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamID = MyTeamID;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
		MyTeamID = NewTeamID;
		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	}
	else
	{
		UE_LOG(LogBETeams, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

FGenericTeamId ABEPlayerState::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnBETeamIndexChangedDelegate* ABEPlayerState::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ABEPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABEPlayerState::OnRep_MySquadID()
{
	//@TODO: Let the squad subsystem know (once that exists)
}

void ABEPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void ABEPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 ABEPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool ABEPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void ABEPlayerState::ClientBroadcastMessage_Implementation(const FBEVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}
