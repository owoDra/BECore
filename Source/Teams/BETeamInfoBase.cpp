// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BETeamInfoBase.h"

#include "Teams/BETeamSubsystem.h"

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Engine/World.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "UObject/CoreNetTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETeamInfoBase)

class FLifetimeProperty;


ABETeamInfoBase::ABETeamInfoBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TeamId(INDEX_NONE)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);
}

void ABETeamInfoBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamTags);
	DOREPLIFETIME_CONDITION(ThisClass, TeamId, COND_InitialOnly);
}

void ABETeamInfoBase::BeginPlay()
{
	Super::BeginPlay();

	TryRegisterWithTeamSubsystem();
}

void ABETeamInfoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TeamId != INDEX_NONE)
	{
		UBETeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UBETeamSubsystem>();
		TeamSubsystem->UnregisterTeamInfo(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ABETeamInfoBase::RegisterWithTeamSubsystem(UBETeamSubsystem* Subsystem)
{
	Subsystem->RegisterTeamInfo(this);
}

void ABETeamInfoBase::TryRegisterWithTeamSubsystem()
{
	if (TeamId != INDEX_NONE)
	{
		UBETeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UBETeamSubsystem>();
		RegisterWithTeamSubsystem(TeamSubsystem);
	}
}

void ABETeamInfoBase::SetTeamId(int32 NewTeamId)
{
	check(HasAuthority());
	check(TeamId == INDEX_NONE);
	check(NewTeamId != INDEX_NONE);

	TeamId = NewTeamId;

	TryRegisterWithTeamSubsystem();
}

void ABETeamInfoBase::OnRep_TeamId()
{
	TryRegisterWithTeamSubsystem();
}
