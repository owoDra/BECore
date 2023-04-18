// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEPawn.h"

#include "BELogChannels.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Controller.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Casts.h"
#include "Trace/Detail/Channel.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPawn)


ABEPawn::ABEPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABEPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ABEPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ABEPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABEPawn::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	// Grab the current team ID and listen for future changes
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABEPawn::UnPossessed()
{
	AController* const OldController = Controller;

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// Determine what the new team ID should be afterwards
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABEPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		}
		else
		{
			UE_LOG(LogBETeams, Error, TEXT("You can't set the team ID on a pawn (%s) except on the authority"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogBETeams, Error, TEXT("You can't set the team ID on a possessed pawn (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

FGenericTeamId ABEPawn::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnBETeamIndexChangedDelegate* ABEPawn::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ABEPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ABEPawn::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
